"""Fail-closed protocol tests; no network, credentials or model calls."""
from copy import deepcopy
from datetime import datetime, timezone
import unittest
from uuid import uuid4
from review_handshake import digest, validate

START = datetime(2026, 11, 3, 15, 0, tzinfo=timezone.utc).timestamp()


def fixture():
    expected = {'project_id':'fiskelaget','repository':'hansarne-lang/fiskespillet','pr_number':23,
                'head_sha':'a'*40,'base_sha':'b'*40,'nonce':'c'*64,'request_id':str(uuid4()),
                'expires_at':datetime.fromtimestamp(START+600,timezone.utc).isoformat(),
                'task_url':'https://github.com/hansarne-lang/fiskespillet/issues/24'}
    payload = {k:v for k,v in expected.items() if k not in ('project_id','task_url')}
    result = dict(payload,verdict='APPROVED',summary='Reviewed actual change; no blocking findings.',findings=[])
    contents = [payload,result,dict(payload,result_message_id=2,result_digest=digest(result))]
    records = []
    for i, (kind,sender,to) in enumerate([('REVIEW_REQUEST','codex','claude'),('REVIEW_RESULT','claude','codex'),('REVIEW_ACK','codex','claude')]):
        import json
        envelope = {'SchemaVersion':'1.0','ProjectId':'fiskelaget','MessageId':expected['request_id'] if i==0 else str(uuid4()),
                    'TaskId':expected['task_url'],'Sender':sender,'Recipient':to,'MessageType':kind,
                    'Timestamp':datetime.fromtimestamp(START+i,timezone.utc).isoformat(),'Payload':contents[i],
                    'RelatedCommit':expected['head_sha'],'PullRequestUrl':'https://github.com/hansarne-lang/fiskespillet/pull/23'}
        records.append({'id':i+1,'from':sender,'to':to,'at':(START+i)*1000,'text':json.dumps(envelope)})
    return {'expected':expected,'current_head_sha':expected['head_sha'],'current_base_sha':expected['base_sha'],
            'messages':records,'reviewer_process':{'provider':'claude-code','exit_code':0,'head_sha':expected['head_sha']},
            'checks':[{'name':'repo-contracts','status':'COMPLETED','conclusion':'SUCCESS','head_sha':expected['head_sha']}]}


def alter_envelope(packet, index, change):
    import json
    message = packet['messages'][index]
    envelope = json.loads(message['text'])
    change(envelope)
    message['text'] = json.dumps(envelope)


class ReviewHandshake(unittest.TestCase):
    def rejected(self, packet, **kwargs):
        with self.assertRaises((ValueError, KeyError, TypeError)):
            validate(packet,now=START+5,**kwargs)

    def test_complete_real_round(self):
        packet=fixture()
        self.assertEqual(validate(packet,now=START+5)['message_ids'],[1,2,3])

    def test_new_head_or_base_requires_new_review(self):
        for name in ['current_head_sha','current_base_sha']:
            with self.subTest(field=name):
                packet=fixture();packet[name]='d'*40;self.rejected(packet)

    def test_wrong_repository_project_pr_nonce_or_sha(self):
        for key,value in [('repository','other/repo'),('pr_number',24),('head_sha','d'*40),('base_sha','e'*40),('nonce','f'*64)]:
            with self.subTest(field=key):
                packet=fixture();alter_envelope(packet,1,lambda e:e['Payload'].update({key:value}));self.rejected(packet)
        packet=fixture();alter_envelope(packet,1,lambda e:e.update(ProjectId='other'));self.rejected(packet)

    def test_missing_result_or_ack_cannot_pass(self):
        for index in [1,2]:
            packet=fixture();del packet['messages'][index];self.rejected(packet)

    def test_self_authored_or_misrouted_result_rejected(self):
        for key,value in [('from','codex'),('to','haj')]:
            packet=fixture();packet['messages'][1][key]=value;self.rejected(packet)
        packet=fixture();alter_envelope(packet,1,lambda e:e.update(Sender='codex'));self.rejected(packet)

    def test_duplicate_result_and_reordered_ack_rejected(self):
        packet=fixture();packet['messages'].append(deepcopy(packet['messages'][1]));self.rejected(packet)
        packet=fixture();packet['messages'][2]['id']=1;self.rejected(packet)

    def test_old_or_consumed_review_rejected(self):
        packet=fixture()
        with self.assertRaises(ValueError):validate(packet,now=START+601)
        self.rejected(packet,used_requests=[packet['expected']['request_id']])

    def test_unbounded_or_future_round_rejected(self):
        packet=fixture();packet['expected']['expires_at']=datetime.fromtimestamp(START+3600,timezone.utc).isoformat();self.rejected(packet)
        packet=fixture();packet['messages'][1]['at']=(START+60)*1000;self.rejected(packet)

    def test_reject_changes_requested_or_blocking_finding(self):
        for verdict in ['CHANGES_REQUESTED','BLOCKED']:
            packet=fixture();alter_envelope(packet,1,lambda e:e['Payload'].update(verdict=verdict));self.rejected(packet)
        for severity,blocking in [('P1',False),('P2',True),('P3',True)]:
            packet=fixture();alter_envelope(packet,1,lambda e:e['Payload'].update(findings=[{'severity':severity,'blocking':blocking,'message':'fix'}]));self.rejected(packet)

    def test_ack_must_name_exact_result_and_digest(self):
        for key,value in [('result_message_id',9),('result_digest','0'*64)]:
            packet=fixture();alter_envelope(packet,2,lambda e:e['Payload'].update({key:value}));self.rejected(packet)

    def test_no_green_status_without_successful_current_ci(self):
        packet=fixture();packet['checks']=[];self.rejected(packet)
        for key,value in [('status','IN_PROGRESS'),('conclusion','FAILURE'),('head_sha','e'*40)]:
            packet=fixture();packet['checks'][0][key]=value;self.rejected(packet)

    def test_reviewer_failure_or_wrong_provenance_rejected(self):
        for key,value in [('provider','codex'),('exit_code',1),('head_sha','e'*40)]:
            packet=fixture();packet['reviewer_process'][key]=value;self.rejected(packet)

    def test_p3_note_can_pass_only_with_matching_ack(self):
        import json
        packet=fixture()
        alter_envelope(packet,1,lambda e:e['Payload'].update(findings=[{'severity':'P3','blocking':False,'message':'Optional wording improvement.'}]))
        result=json.loads(packet['messages'][1]['text'])['Payload']
        alter_envelope(packet,2,lambda e:e['Payload'].update(result_digest=digest(result)))
        self.assertEqual(validate(packet,now=START+5)['verdict'],'APPROVED')


if __name__=='__main__':
    unittest.main()
