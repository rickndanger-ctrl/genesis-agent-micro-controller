#!/usr/bin/env python3
import hashlib,json,pathlib,shutil,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
VERSION='0.4.0-preview'
DIST=ROOT/'dist';stage=DIST/f'Genesis-Agent-Micro-Controller-v{VERSION}'
if stage.exists():shutil.rmtree(stage)
stage.mkdir(parents=True)
for name in ['README.md','AGENT_SETUP.md','LICENSE','NOTICE.md','docs','scripts','firmware','macos','tests','licenses']:
 src=ROOT/name
 if src.is_dir():shutil.copytree(src,stage/name,ignore=shutil.ignore_patterns('*.app','build','__pycache__','*.pyc'))
 else:shutil.copy2(src,stage/name)
image=ROOT/'build/firmware/GenesisMicro.ino.merged.bin'
if image.stat().st_size!=4194304:raise SystemExit('Expected a merged 4MB image')
shutil.copy2(image,stage/'genesis-mini.bin')
subprocess.run(['/usr/bin/ditto',str(ROOT/'macos/Genesis Agent Micro Controller.app'),str(stage/'Genesis Agent Micro Controller.app')],check=True)
manifest={'version':VERSION,'board':'axiometa_genesis_mini','chip':'esp32s3','flash_bytes':4194304,'image':'genesis-mini.bin','sha256':hashlib.sha256(image.read_bytes()).hexdigest()}
(stage/'firmware-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
lines=[]
for path in sorted(stage.rglob('*')):
 if path.is_file():lines.append(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+str(path.relative_to(stage)))
(stage/'SHA256SUMS').write_text('\n'.join(lines)+'\n')
archive=DIST/(stage.name+'.zip')
if archive.exists():archive.unlink()
subprocess.run(['/usr/bin/ditto','-c','-k','--keepParent',str(stage),str(archive)],check=True)
print(archive)
