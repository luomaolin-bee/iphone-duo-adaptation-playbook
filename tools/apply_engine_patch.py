#!/usr/bin/env python3
import argparse,json,struct,hashlib
from pathlib import Path
from common import ROOT,state,macho_identity

def main():
 p=argparse.ArgumentParser();p.add_argument('--engine',type=Path,required=True);p.add_argument('--out',type=Path,required=True);args=p.parse_args()
 if args.engine.resolve()==args.out.resolve():raise SystemExit('Use a separate output file; input is preserved.')
 original=args.engine.read_bytes();config=state();source=config['historical_engine_input'];target=config['package_baselines'][0]
 if hashlib.sha256(original).hexdigest()!=source['engine_sha256']:raise SystemExit('Expected the exact 0.2.14 recovery engine; input SHA-256 mismatch.')
 image=macho_identity(original)
 if image['uuid_hex']!=source['engine_uuid_hex']:raise SystemExit('Engine UUID mismatch.')
 manifest=json.loads((ROOT/'manifests/engine-patches-0.2.16.json').read_text());data=bytearray(original);allowed=set()
 for edit in manifest['patches']:
  offset=int(edit['offset'],16);before=bytes.fromhex(edit['before']);after=bytes.fromhex(edit['after'])
  if len(before)!=len(after)or original[offset:offset+len(before)]!=before:raise SystemExit(f'Expected-byte mismatch at {offset:#x}.')
  if allowed.intersection(range(offset,offset+len(after))):raise SystemExit('Overlapping manifest edits.')
  data[offset:offset+len(after)]=after;allowed.update(range(offset,offset+len(after)))
 shared=int(manifest['stateAddress'],16);valid=False
 for offset,size in image['segments']:
  if data[offset+8:offset+24].split(b'\0')[0]!=b'__DATA':continue
  vm,length=struct.unpack_from('<QQ',data,offset+24);end=vm
  for i in range(struct.unpack_from('<I',data,offset+64)[0]):
   address,n=struct.unpack_from('<QQ',data,offset+72+i*80+32);end=max(end,address+n)
  valid=shared>=end and shared+12<=vm+length and bool(struct.unpack_from('<I',data,offset+60)[0]&2)
 if not valid:raise SystemExit('Shared viewport state does not fit reviewed writable padding.')
 if any(data[0x1800:0x1838])or any(data[0x1900:0x1a00]):raise SystemExit('Withdrawn header helper ranges are not empty.')
 if any(i not in allowed for i,(a,b)in enumerate(zip(original,data))if a!=b):raise SystemExit('Unlisted binary changes.')
 result=hashlib.sha256(data).hexdigest()
 if result!=target['engine_sha256']:raise SystemExit('Output does not match the archived 0.2.16+ engine SHA-256.')
 args.out.parent.mkdir(parents=True,exist_ok=True);args.out.write_bytes(data);print(json.dumps({'output_sha256':result,'ranges':len(manifest['patches']),'input_preserved':True},indent=2))
if __name__=='__main__':main()
