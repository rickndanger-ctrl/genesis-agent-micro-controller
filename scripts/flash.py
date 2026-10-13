#!/usr/bin/env python3
"""Agent-assisted installer. No writes until target, image, and backup checks pass."""
import argparse, hashlib, json, pathlib, re, subprocess, sys
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def normalized_mac(value):
    if not re.fullmatch(r'(?:[0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}', value):
        raise ValueError('Expected a six-byte colon-separated MAC address')
    return value.lower()
def checked_image(root):
    manifest=json.loads((root/'firmware-manifest.json').read_text())
    if manifest.get('chip')!='esp32s3' or manifest.get('board')!='axiometa_genesis_mini' or manifest.get('flash_bytes')!=4194304:
        raise ValueError('Unsupported firmware manifest')
    image=(root/manifest['image']).resolve()
    if not image.is_relative_to(root.resolve()) or image.stat().st_size!=4194304 or sha(image)!=manifest['sha256']:
        raise ValueError('Firmware missing, size invalid, or SHA-256 mismatch')
    return image

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--list',action='store_true',help='List USB serial candidates without writing')
    p.add_argument('--port');p.add_argument('--mac',help='Exact ROM-mode serial/MAC of the intended board')
    p.add_argument('--confirm-genesis-mini',action='store_true')
    p.add_argument('--confirm-replace-firmware',action='store_true')
    p.add_argument('--backup-dir',type=pathlib.Path,default=pathlib.Path.home()/'GenesisBackups')
    a=p.parse_args()
    from serial.tools import list_ports
    ports=list(list_ports.comports())
    if a.list:
        for port in ports:
            if port.vid==0x303a: print(json.dumps({'port':port.device,'vid':port.vid,'pid':port.pid,'serial':port.serial_number,'description':port.description}))
        return
    if not (a.port and a.mac and a.confirm_genesis_mini and a.confirm_replace_firmware):
        p.error('Specify --port, --mac, --confirm-genesis-mini and --confirm-replace-firmware after identifying the board and obtaining user approval')
    expected=normalized_mac(a.mac)
    matches=[v for v in ports if v.device==a.port and v.vid==0x303a and v.pid==0x1001 and (v.serial_number or '').lower()==expected]
    if len(matches)!=1: raise ValueError('Exact board not found in ROM mode; hold BOOT, tap RESET, release BOOT and list devices again')
    image=checked_image(ROOT)
    base=[sys.executable,'-m','esptool','--chip','esp32s3','--port',a.port]
    def run(args):
        r=subprocess.run(base+args,text=True,capture_output=True,check=True)
        print(r.stdout,flush=True);return r.stdout
    probe=run(['read-mac'])
    if not re.search(r'(?im)^MAC:\s*'+re.escape(expected)+r'\s*$',probe): raise ValueError('Live MAC differs; refusing to write')
    info=run(['flash-id'])
    if not re.search(r'Detected flash size:\s*4MB\b',info,re.I): raise ValueError('This release supports the tested 4MB Genesis Mini only')
    a.backup_dir.mkdir(parents=True,exist_ok=True)
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    backup=a.backup_dir/('genesis-'+expected.replace(':','')+'-'+stamp+'.bin')
    if backup.exists():raise ValueError('Backup collision')
    run(['read-flash','0','0x400000',str(backup)])
    if backup.stat().st_size!=4194304: raise ValueError('Backup incomplete; refusing to write')
    # Verify the backup against the live flash, not just against its own checksum.
    run(['verify-flash','0',str(backup)])
    receipt={'mac':expected,'port':a.port,'backup':str(backup),'backup_sha256':sha(backup),'image_sha256':sha(image),'status':'backup-verified'}
    receipt_path=backup.with_suffix('.json');receipt_path.write_text(json.dumps(receipt,indent=2))
    # A reconnect or swapped device must pass identification again immediately before write.
    probe=run(['read-mac'])
    if not re.search(r'(?im)^MAC:\s*'+re.escape(expected)+r'\s*$',probe):raise ValueError('Target changed after backup')
    run(['--baud','460800','write-flash','0',str(image)])
    receipt['status']='write-and-readback-hash-verified';receipt_path.write_text(json.dumps(receipt,indent=2))
    print('Installed and verified. Tap RESET without BOOT. Keep your backup private; it may contain prior device data.')

if __name__=='__main__':
    try:main()
    except (ValueError,OSError,subprocess.CalledProcessError) as e:
        print('STOP: '+str(e),file=sys.stderr);sys.exit(1)
