"""Validate a bounded Broen review round before a trusted coordinator posts status.

Broen's local transport and the coordinator are the trust boundary. This does not
prove a separate GitHub identity or turn an arbitrary JSON file into approval.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import time
from uuid import UUID


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode('utf-8')).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def timestamp(value):
    require(isinstance(value, str), 'Timestamp must be text')
    parsed = datetime.fromisoformat(value.replace('Z', '+00:00'))
    require(parsed.tzinfo is not None, 'Timestamp needs timezone')
    return parsed.timestamp()


def validate(packet, *, now=None, used_requests=()):
    """Validate transport records plus fresh GitHub facts supplied by coordinator.

    Caller must fetch current head/base and CI itself, preserve broker records,
    serialize publication, and durably consume the request before allowing reuse.
    """
    now = time.time() if now is None else now
    expected = packet['expected']
    require(expected['project_id'] == 'fiskelaget', 'Wrong project')
    require(expected['repository'] == 'hansarne-lang/fiskespillet', 'Wrong repository')
    require(type(expected['pr_number']) is int and expected['pr_number'] > 0, 'Invalid PR')
    require(all(re.fullmatch('[0-9a-f]{40}', expected[k]) for k in ('head_sha', 'base_sha')), 'Full SHA required')
    require(re.fullmatch('[0-9a-f]{64}', expected['nonce']) is not None, 'Invalid nonce')
    UUID(expected['request_id'])
    require(expected['request_id'] not in used_requests, 'Request already consumed')
    require(packet['current_head_sha'] == expected['head_sha'], 'Head changed; new review required')
    require(packet['current_base_sha'] == expected['base_sha'], 'Base changed; new review required')
    expires = timestamp(expected['expires_at'])
    require(now <= expires, 'Expired review round')

    matching = []
    for record in packet['messages']:
        try:
            envelope = json.loads(record['text'])
        except (ValueError, TypeError):
            continue
        if isinstance(envelope, dict) and isinstance(envelope.get('Payload'), dict) and envelope['Payload'].get('request_id') == expected['request_id']:
            matching.append((record, envelope))

    chain = []
    for kind, sender, recipient in [('REVIEW_REQUEST', 'codex', 'claude'), ('REVIEW_RESULT', 'claude', 'codex'), ('REVIEW_ACK', 'codex', 'claude')]:
        candidates = [(record, envelope) for record, envelope in matching if envelope.get('MessageType') == kind]
        require(len(candidates) == 1, 'Missing or duplicate ' + kind)
        record, envelope = candidates[0]
        require(record['from'] == sender and record['to'] == recipient, 'Wrong transport sender/recipient')
        require(envelope.get('Sender') == sender and envelope.get('Recipient') == recipient, 'Wrong envelope sender/recipient')
        require(envelope.get('SchemaVersion') == '1.0', 'Unsupported envelope version')
        require(envelope.get('ProjectId') == expected['project_id'], 'Envelope project mismatch')
        require(envelope.get('TaskId') == expected['task_url'], 'Task mismatch')
        require(envelope.get('RelatedCommit') == expected['head_sha'], 'Related commit mismatch')
        require(envelope.get('PullRequestUrl') == f"https://github.com/{expected['repository']}/pull/{expected['pr_number']}", 'PR URL mismatch')
        UUID(envelope['MessageId'])
        require(type(record['id']) is int and record['id'] > 0, 'Invalid transport ID')
        when = record['at'] / 1000
        require(timestamp(envelope['Timestamp']) <= now + 30, 'Future envelope')
        require(abs(timestamp(envelope['Timestamp']) - when) <= 30, 'Envelope/transport time mismatch')
        require(when <= min(expires, now + 30), 'Late or future transport record')
        require(all(envelope['Payload'].get(k) == expected[k] for k in ('repository', 'pr_number', 'head_sha', 'base_sha', 'nonce', 'request_id', 'expires_at')), 'Review correlation mismatch')
        chain.append((record, envelope))

    ids = [record['id'] for record, _ in chain]
    dates = [record['at'] / 1000 for record, _ in chain]
    require(ids[0] < ids[1] < ids[2] and dates[0] <= dates[1] <= dates[2], 'Review/ack order invalid')
    require(len({e['MessageId'] for _, e in chain}) == 3, 'Reused message UUID')
    require(chain[0][1]['MessageId'] == expected['request_id'], 'Request UUID mismatch')
    require(0 < expires - dates[0] <= 15 * 60, 'Unbounded review round')

    result = chain[1][1]['Payload']
    ack = chain[2][1]['Payload']
    require(result.get('verdict') == 'APPROVED', 'Claude did not approve')
    findings = result.get('findings')
    require(isinstance(findings, list), 'Missing findings list')
    for finding in findings:
        require(isinstance(finding, dict), 'Invalid finding')
        require(finding.get('severity') in ('P0', 'P1', 'P2', 'P3'), 'Invalid finding severity')
        require(type(finding.get('blocking')) is bool, 'Missing blocking classification')
        require(not finding['blocking'] and finding['severity'] == 'P3', 'Blocking finding remains')
    require(isinstance(result.get('summary'), str) and result['summary'].strip(), 'Missing review summary')
    require(ack.get('result_message_id') == ids[1], 'Acknowledged wrong result')
    require(ack.get('result_digest') == digest(result), 'Acknowledged different result content')

    require(packet.get('reviewer_process', {}).get('provider') == 'claude-code', 'Missing real reviewer provenance')
    require(packet['reviewer_process'].get('exit_code') == 0, 'Reviewer process failed')
    require(packet['reviewer_process'].get('head_sha') == expected['head_sha'], 'Reviewer process SHA mismatch')
    checks = [c for c in packet['checks'] if c.get('name') == 'repo-contracts']
    require(checks, 'Missing required CI check')
    require(all(c.get('head_sha') == expected['head_sha'] and c.get('status') == 'COMPLETED' and c.get('conclusion') == 'SUCCESS' for c in checks), 'Required CI failed, pending or stale')

    return {'schema_version':'1.0', 'project_id':expected['project_id'], 'repository':expected['repository'],
            'pr_number':expected['pr_number'], 'head_sha':expected['head_sha'], 'base_sha':expected['base_sha'],
            'request_id':expected['request_id'], 'nonce_digest':digest(expected['nonce']),
            'message_ids':ids, 'result_digest':digest(result), 'verdict':'APPROVED',
            'verified_at':datetime.fromtimestamp(now, timezone.utc).isoformat(),
            'reviewer':'Claude Code via isolated Broen', 'summary':result['summary'], 'findings':findings}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('packet', type=Path)
    parser.add_argument('--consumed', type=Path, help='Private coordinator ledger of consumed request IDs')
    args = parser.parse_args()
    packet = json.loads(args.packet.read_text(encoding='utf-8'))
    used = json.loads(args.consumed.read_text(encoding='utf-8')) if args.consumed and args.consumed.exists() else []
    try:
        receipt = validate(packet, used_requests=used)
    except (ValueError, KeyError, TypeError) as exc:
        raise SystemExit('Handshake rejected: ' + str(exc))
    print(json.dumps(receipt, ensure_ascii=False, indent=2))
