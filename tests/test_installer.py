import hashlib, importlib.util, json, pathlib, tempfile, unittest
spec=importlib.util.spec_from_file_location('flash',pathlib.Path(__file__).parents[1]/'scripts/flash.py')
flash=importlib.util.module_from_spec(spec);spec.loader.exec_module(flash)
class InstallerTests(unittest.TestCase):
 def test_mac(self):
  self.assertEqual(flash.normalized_mac('AA:BB:CC:DD:EE:FF'),'aa:bb:cc:dd:ee:ff')
  with self.assertRaises(ValueError):flash.normalized_mac('random')
 def test_image_and_corruption(self):
  with tempfile.TemporaryDirectory() as d:
   r=pathlib.Path(d);image=r/'firmware.bin';image.write_bytes(bytes(4194304))
   m={'chip':'esp32s3','board':'axiometa_genesis_mini','flash_bytes':4194304,'image':'firmware.bin','sha256':flash.sha(image)}
   (r/'firmware-manifest.json').write_text(json.dumps(m));self.assertEqual(flash.checked_image(r),image.resolve())
   image.write_bytes(b'bad')
   with self.assertRaises(ValueError):flash.checked_image(r)
 def test_wrong_board(self):
  with tempfile.TemporaryDirectory() as d:
   r=pathlib.Path(d);(r/'firmware-manifest.json').write_text(json.dumps({'chip':'esp32'}))
   with self.assertRaises(ValueError):flash.checked_image(r)
if __name__=='__main__':unittest.main()

class FlashSequenceTests(unittest.TestCase):
 def run_case(self, bad_mac=False, bad_size=False):
  from unittest.mock import patch
  import types,sys,io,contextlib
  with tempfile.TemporaryDirectory() as d:
   r=pathlib.Path(d);image=r/'firmware.bin';image.write_bytes(bytes(4194304))
   (r/'firmware-manifest.json').write_text(json.dumps({'chip':'esp32s3','board':'axiometa_genesis_mini','flash_bytes':4194304,'image':'firmware.bin','sha256':flash.sha(image)}))
   port=types.SimpleNamespace(device='/dev/fake',vid=0x303a,pid=0x1001,serial_number='aa:bb:cc:dd:ee:ff')
   serial=types.ModuleType('serial');serial_tools=types.ModuleType('serial.tools');serial_tools.list_ports=types.SimpleNamespace(comports=lambda:[port])
   commands=[]
   def run(args,**kwargs):
    commands.append(args)
    if 'read-mac' in args:out='MAC: '+('00:00:00:00:00:00' if bad_mac else port.serial_number)+'\n'
    elif 'flash-id' in args:out='Detected flash size: '+('8MB' if bad_size else '4MB')+'\n'
    elif 'read-flash' in args:pathlib.Path(args[-1]).write_bytes(bytes(4194304));out='Read complete'
    else:out='Verified'
    return types.SimpleNamespace(stdout=out)
   argv=['flash.py','--port',port.device,'--mac',port.serial_number,'--confirm-genesis-mini','--confirm-replace-firmware','--backup-dir',str(r/'backups')]
   with patch.dict(sys.modules,{'serial':serial,'serial.tools':serial_tools}),patch.object(flash,'ROOT',r),patch.object(sys,'argv',argv),patch.object(flash.subprocess,'run',side_effect=run),contextlib.redirect_stdout(io.StringIO()):
    if bad_mac or bad_size:
     with self.assertRaises(ValueError):flash.main()
    else:flash.main()
   operations=[next(x for x in a if x in ['read-mac','flash-id','read-flash','verify-flash','write-flash']) for a in commands]
   return operations
 def test_backup_before_write(self):
  self.assertEqual(self.run_case(),['read-mac','flash-id','read-flash','verify-flash','read-mac','write-flash'])
 def test_wrong_mac_never_writes(self):self.assertNotIn('write-flash',self.run_case(bad_mac=True))
 def test_wrong_flash_never_writes(self):self.assertNotIn('write-flash',self.run_case(bad_size=True))
