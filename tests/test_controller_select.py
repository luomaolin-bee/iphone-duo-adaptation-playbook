import re,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'src'
class ControllerSelectIntegrationTests(unittest.TestCase):
 def setUp(self):
  self.bridge=(SRC/'DuoSceneBridge.m').read_text();self.module=(SRC/'ControllerSelect.inc').read_text();self.controls=(SRC/'Controls.inc').read_text()
 def test_included_once_after_controls(self):
  self.assertEqual(self.bridge.count('#include "ControllerSelect.inc"'),1)
  self.assertLess(self.bridge.index('#include "Controls.inc"'),self.bridge.index('#include "ControllerSelect.inc"'))
 def test_installed_and_reported(self):
  self.assertIn('InstallControllerSelect();',self.bridge);self.assertIn('report[@"controllerSelect"]=ControllerSelectReport();',self.bridge)
 def test_framework_linked(self):
  self.assertIn("'GameController'",(ROOT/'tools/build.py').read_text())
 def test_no_game_handler_replaced(self):
  for forbidden in ['pressedChangedHandler','valueChangedHandler','controllerPausedHandler']:self.assertNotIn(forbidden+'=',self.module.replace(' ',''))
 def test_home_not_touched(self):
  self.assertNotIn('buttonHome',self.module)
 def test_only_existing_controls_pulsed(self):
  self.assertIn('PulseControl(pad,fire)',self.module);self.assertNotRegex(self.module,r'PulseControl\(pad,\s*(?!fire)')
 def test_pulse_bypass_precedes_menu_hiding(self):
  hook=self.bridge[self.bridge.index('static BOOL HiddenHook'):]
  self.assertLess(hook.index('if(pulseInput&&control==pulseControl)return NO;'),hook.index('if(MenuHides(control))return YES;'))
 def test_menu_entry_present(self):
  self.assertIn('ControllerSelectMenuTitle()',self.controls);self.assertIn('CycleControllerSelectMode();',self.controls)
 def test_preference_keys_preserved(self):
  for key in ['DuoUnifiedLayouts018','DuoConsoleGlass017','DuoFPS018','DuoAutoMenus018','DuoReflectionQuality018','DuoFeedback019']:
   self.assertTrue(any(key in f.read_text() for f in SRC.iterdir() if f.suffix in {'.m','.inc'}),key)
  self.assertIn('DuoControllerSelect021',self.module)
if __name__=='__main__':unittest.main()
