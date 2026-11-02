from pathlib import Path
import hashlib,json,struct
ROOT=Path(__file__).resolve().parents[1]
APP='Payload/SilksongNative.app/'
ENGINE=APP+'Frameworks/UnityFramework.framework/UnityFramework'
BRIDGE=APP+'Frameworks/DuoSceneBridge.dylib'
ART=APP+'FakeLuStudio.jpg'
def state():return json.loads((ROOT/'PROJECT_STATE.json').read_text())
def digest_stream(stream):
 h=hashlib.sha256()
 for b in iter(lambda:stream.read(1024*1024),b''):h.update(b)
 return h.hexdigest()
def digest_file(path):
 with Path(path).open('rb')as f:return digest_stream(f)
def macho_identity(data):
 if len(data)<32 or data[:4]!=b'\xcf\xfa\xed\xfe':raise ValueError('Expected a thin little-endian Mach-O 64 image')
 cpu=struct.unpack_from('<I',data,4)[0];count,command_bytes=struct.unpack_from('<II',data,16)
 if 32+command_bytes>len(data):raise ValueError('Truncated load-command region')
 pos=32;uuid=None;install_name=None;segments=[]
 for _ in range(count):
  if pos+8>32+command_bytes:raise ValueError('Truncated load command')
  cmd,size=struct.unpack_from('<II',data,pos)
  if size<8 or size%4 or pos+size>32+command_bytes:raise ValueError('Invalid load-command size')
  if cmd==0x1b:
   if size<24:raise ValueError('Invalid UUID command')
   uuid=data[pos+8:pos+24].hex()
  if cmd==0xd:
   offset=struct.unpack_from('<I',data,pos+8)[0]
   if offset>=size:raise ValueError('Invalid dylib ID')
   install_name=data[pos+offset:pos+size].split(b'\0')[0].decode()
  if cmd==0x19:segments.append((pos,size))
  pos+=size
 if pos!=32+command_bytes:raise ValueError('Load-command count mismatch')
 return {'cpu':cpu,'uuid_hex':uuid,'install_name':install_name,'segments':segments}
