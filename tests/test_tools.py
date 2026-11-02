import sys,unittest,tempfile,struct,uuid,zipfile,plistlib
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from common import APP,ENGINE,BRIDGE,ART,digest_file,macho_identity
from package import package

def image(dylib=False):
 command=struct.pack('<II',0x1b,24)+uuid.UUID('ad0a91df-b78c-38bc-8fb7-0aeba6d2b6a5').bytes
 count=1
 if dylib:
  name=b'@rpath/DuoSceneBridge.dylib\0';size=(24+len(name)+7)&~7
  command+=(struct.pack('<6I',0xd,size,24,0,0,0)+name).ljust(size,b'\0');count+=1
 return struct.pack('<IiiIIIII',0xfeedfacf,0x100000c,0,6 if dylib else 2,count,len(command),0,0)+command
class ToolsTests(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name);self.base=self.root/'baseline.ipa';self.bridge=self.root/'bridge.dylib';self.art=self.root/'studio.jpg';self.out=self.root/'out.ipa'
  self.bridge.write_bytes(image(True));self.art.write_bytes(b'test artwork fixture');self.engine=image()
  with zipfile.ZipFile(self.base,'w')as z:
   z.writestr(ENGINE,self.engine);z.writestr(BRIDGE,b'old bridge');z.writestr(APP+'game-data.txt',b'preserve data')
   z.writestr(APP+'Info.plist',plistlib.dumps({'UIApplicationSceneManifest':{'UISceneConfigurations':{'UIWindowSceneSessionRoleApplication':[{'UISceneDelegateClassName':'DuoSceneDelegate'}]}}}))
  import hashlib
  self.identities={'package_baselines':[{'ipa_sha256':digest_file(self.base),'engine_uuid_hex':macho_identity(self.engine)['uuid_hex'],'engine_sha256':hashlib.sha256(self.engine).hexdigest()}]}
 def tearDown(self):self.temp.cleanup()
 def test_valid_package_only_bridge_and_art(self):
  r=package(self.base,self.bridge,self.out,self.art,self.identities)
  self.assertEqual(r['changed_entries'],[BRIDGE]);self.assertEqual(r['added_entries'],[ART]);self.assertFalse(r['device_execution_verified'])
  with zipfile.ZipFile(self.out)as z:self.assertEqual(z.read(ENGINE),self.engine);self.assertEqual(z.read(APP+'game-data.txt'),b'preserve data')
 def test_reject_unknown_baseline_before_write(self):
  self.identities['package_baselines'][0]['ipa_sha256']='0'*64
  with self.assertRaisesRegex(ValueError,'Unrecognized'):package(self.base,self.bridge,self.out,self.art,self.identities)
  self.assertFalse(self.out.exists())
 def test_reject_different_engine_even_with_known_archive(self):
  self.identities['package_baselines'][0]['engine_sha256']='0'*64
  with self.assertRaisesRegex(ValueError,'Engine bytes'):package(self.base,self.bridge,self.out,self.art,self.identities)
 def test_reject_bad_uuid(self):
  self.identities['package_baselines'][0]['engine_uuid_hex']='0'*32
  with self.assertRaisesRegex(ValueError,'UUID'):package(self.base,self.bridge,self.out,self.art,self.identities)
 def test_reject_wrong_library_identity(self):
  self.bridge.write_bytes(image(False))
  with self.assertRaisesRegex(ValueError,'install name'):package(self.base,self.bridge,self.out,self.art,self.identities)
 def test_do_not_overwrite_baseline(self):
  before=self.base.read_bytes()
  with self.assertRaisesRegex(ValueError,'overwrite'):package(self.base,self.bridge,self.base,self.art,self.identities)
  self.assertEqual(self.base.read_bytes(),before)
 def test_malformed_command_not_accepted(self):
  bad=bytearray(image());struct.pack_into('<I',bad,36,0)
  with self.assertRaises(ValueError):macho_identity(bad)
 def test_reject_truncated_macho(self):
  with self.assertRaises(ValueError):macho_identity(image()[:-2])
 def test_reject_non_macho(self):
  with self.assertRaises(ValueError):macho_identity(b'not a mach-o')
if __name__=='__main__':unittest.main()
