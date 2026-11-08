"""Static project invariants only; this does not compile or launch Unreal."""
import configparser
import json
from pathlib import Path
import re
import sys


def dependencies(source):
    values = []
    for match in re.finditer(r'(?:Public|Private)DependencyModuleNames\.(?:Add|AddRange)\((.*?)\);', source, re.S):
        values.extend(re.findall(r'"([^"\n]+)"', match[1]))
    return set(values)


def validate(root):
    root = Path(root)
    errors = []
    try:
        project = json.loads((root/'FiskELaget.uproject').read_text(encoding='utf-8'))
        modules = project['Modules']
        if len(modules) != 2 or {m['Name'] for m in modules} != {'FiskCore','FiskGame'}:
            errors.append('Expected exactly FiskCore and FiskGame')
        if any(m['Type'] != 'Runtime' for m in modules):
            errors.append('Both foundation modules must be Runtime')
        if project.get('EngineAssociation') != '5.8':
            errors.append('Expected portable UE 5.8 association')
        renderer = [p for p in project.get('Plugins', []) if p.get('Name') == 'ProceduralMeshComponent']
        if len(renderer) != 1 or renderer[0].get('Enabled') is not True:
            errors.append('ProceduralMeshComponent must be explicitly enabled')
    except (OSError,ValueError,KeyError,TypeError):
        errors.append('Invalid or missing uproject')
    expected = {'FiskCore':{'Core','CoreUObject'}, 'FiskGame':{'Core','CoreUObject','Engine','FiskCore','InputCore','ProceduralMeshComponent'}}
    for name, allowed in expected.items():
        path = root/'Source'/name/(name+'.Build.cs')
        if not path.is_file():
            errors.append('Missing ModuleRules: '+name)
        elif dependencies(path.read_text(encoding='utf-8')) != allowed:
            errors.append('Invalid dependency direction/set: '+name)
    for name, kind in [('FiskELaget','Game'),('FiskELagetEditor','Editor')]:
        path=root/'Source'/(name+'.Target.cs')
        text=path.read_text(encoding='utf-8') if path.exists() else ''
        for token in ['class '+name+'Target', 'Type = TargetType.'+kind, 'BuildSettingsVersion.V7', 'EngineIncludeOrderVersion.Unreal5_8', 'ExtraModuleNames.Add("FiskGame")']:
            if token not in text:
                errors.append(name+': missing '+token)
    for relative in ['Source/FiskCore/Private/FiskCoreModule.cpp','Source/FiskGame/Private/FiskGameModule.cpp',
                     'Source/FiskGame/Private/Bootstrap/FiskBootstrapGameMode.h','Source/FiskGame/Private/Bootstrap/FiskBootstrapGameMode.cpp',
                     'Source/FiskGame/Private/Bootstrap/FiskBootstrapHUD.h','Source/FiskGame/Private/Bootstrap/FiskBootstrapHUD.cpp']:
        if not (root/relative).is_file():
            errors.append('Missing source: '+relative)
    for path in (root/'Source'/'FiskCore').rglob('*'):
        if path.suffix.lower() in ('.cpp','.h') and re.search(r'#include\s*[<"][^\n"]*(?:FiskGame|GameFramework|Engine/)',path.read_text(encoding='utf-8')):
            errors.append('FiskCore includes higher-layer code: '+path.name)
    try:
        config=configparser.ConfigParser(interpolation=None,strict=False)
        config.read(root/'Config'/'DefaultEngine.ini',encoding='utf-8')
        section=config['/Script/EngineSettings.GameMapsSettings']
        if section.get('GameDefaultMap') != '/Engine/Maps/Entry':
            errors.append('Expected engine Entry map for bootstrap')
        if section.get('GlobalDefaultGameMode') != '/Script/FiskGame.FiskBootstrapGameMode':
            errors.append('Missing bootstrap GameMode configuration')
    except (OSError,KeyError,configparser.Error):
        errors.append('Invalid game-map configuration')
    return errors


if __name__=='__main__':
    errors=validate(Path(__file__).resolve().parents[2])
    print(json.dumps({'check':'bootstrap-structure','status':'failed' if errors else 'passed','errors':errors,'unreal_build':'Not Run by this check'},ensure_ascii=False,indent=2))
    sys.exit(bool(errors))
