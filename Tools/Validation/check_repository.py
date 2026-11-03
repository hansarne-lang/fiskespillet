"""Dependency-free checks for the FISKELAGET public repository."""
from pathlib import Path
import json
import re
import sys
from urllib.parse import unquote

EVENTS = ('FishHooked','FishFightStarted','CatchLanded','CatchStored','TowingStarted',
          'ForcedReturnStarted','VoluntaryReturnStarted','BoatArrived',
          'DayDeadlineReached','CatchSold','CatchLost','CareerUpdated')
STATUSES = ['Backlog','Ready','In Progress','Blocked','In Review','Done']
REQUIRED = ('README.md','ARCHITECTURE.md','MODULE_CONTRACTS.md','AGENTS.md','CLAUDE.md',
            'TASK_SYSTEM.md','TESTING.md','DEVELOPMENT.md','CONTRIBUTING.md',
            'AI_BRIDGE.md','SECURITY.md','DECISIONS.md','CHANGELOG.md',
            'Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.0.md',
            '.gitattributes','.gitignore','.github/CODEOWNERS',
            '.github/project-blueprint.json','.github/workflows/repository.yml')

def validate(root):
    root=Path(root).resolve()
    errors=[]
    for name in REQUIRED:
        if not (root/name).is_file(): errors.append('Missing required file: '+name)
    contracts=root/'MODULE_CONTRACTS.md'
    if contracts.is_file():
        content=contracts.read_text(encoding='utf-8')
        for name in EVENTS:
            pattern=r'^### '+re.escape(name)+r'\s*\n(.*?)(?=^### |^## |\Z)'
            match=re.search(pattern,content,re.M|re.S)
            if not match: errors.append('Missing event: '+name);continue
            for label in ('Eier/formål:','Felter:','Avsender:','Konsumenter:','Autoritet/feil:','Versjonering:'):
                if label not in match[1]: errors.append(name+': missing '+label)
    blueprint=root/'.github/project-blueprint.json'
    if blueprint.is_file():
        try:
            data=json.loads(blueprint.read_text(encoding='utf-8'))
            if data['fields']['Status']!=STATUSES: errors.append('Invalid task status contract')
            if 'Awaiting Approval' not in data['fields']['Approval']: errors.append('Missing approval gate')
            if data['automatic_closed_issue_to_done']: errors.append('Unsafe automatic Done transition')
        except (ValueError,KeyError,TypeError): errors.append('Invalid Project blueprint')
    expected={'responsible','priority_request','module_request','dependencies','allowed_files','acceptance','tests'}
    forms=list((root/'.github/ISSUE_TEMPLATE').glob('[123]-*.yml'))
    if len(forms)!=3: errors.append('Expected three Issue forms')
    for form in forms:
        try:
            data=json.loads(form.read_text(encoding='utf-8'))  # JSON is a YAML subset.
            ids=[element['id'] for element in data['body'] if 'id' in element]
            if len(ids)!=len(set(ids)): errors.append(form.name+': duplicate field id')
            if not expected.issubset(ids): errors.append(form.name+': missing task fields')
            if not data.get('name') or not data.get('description'): errors.append(form.name+': missing metadata')
        except (ValueError,KeyError,TypeError): errors.append(form.name+': invalid form structure')
    for suffix in ('uasset','umap'):
        attrs=(root/'.gitattributes').read_text(encoding='utf-8') if (root/'.gitattributes').exists() else ''
        line=next((line for line in attrs.splitlines() if line.startswith('*.'+suffix+' ')), '')
        if 'filter=lfs' not in line or 'lockable' not in line: errors.append('Missing lockable LFS rule: '+suffix)
    for path in root.rglob('*'):
        if '.git' in path.relative_to(root).parts or '__pycache__' in path.parts: continue
        if path.is_symlink(): errors.append('Symlink requires explicit review: '+str(path.relative_to(root)));continue
        if not path.is_file(): continue
        relative=path.relative_to(root).as_posix()
        if path.name in ('.env','hosts.yml','state.json') or path.suffix.lower() in ('.pem','.pfx','.key'):
            errors.append('Sensitive/local file: '+relative)
        if path.suffix.lower() not in ('.md','.py','.json','.yml','.yaml','.txt'):continue
        try: content=path.read_text(encoding='utf-8')
        except UnicodeError: errors.append('Invalid UTF-8: '+relative);continue
        if re.search(r'gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}|-----BEGIN [A-Z ]*PRIVATE KEY-----',content):
            errors.append('Possible credential in '+relative)
        if re.search(r'chatgpt(?:\.com|\.conversation)[/:][^\s]*[0-9a-f]{8}-[0-9a-f-]{27,}',content,re.I):
            errors.append('Private conversation reference in '+relative)
        if re.search(r'[A-Za-z]:[\\/]+Users[\\/]',content): errors.append('Personal local path in '+relative)
        if path.suffix.lower()=='.md':
            for link in re.findall(r'\]\(([^)]+)\)',content):
                link=link.split()[0].strip('<>')
                if re.match(r'^[a-z][a-z0-9+.-]*:',link,re.I) or link.startswith('#'):continue
                dest=(path.parent/unquote(link.split('#')[0])).resolve()
                if not dest.is_relative_to(root) or not dest.exists():errors.append(relative+': broken local link '+link)
    return errors

if __name__=='__main__':
    directory=Path(__file__).resolve().parents[2]
    problems=validate(directory)
    print(json.dumps({'check':'repo-contracts','errors':problems,'status':'failed' if problems else 'passed'},ensure_ascii=False,indent=2))
    sys.exit(bool(problems))
