"""Exercise actual menu resolver/binding with synthetic GameController objects."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]

class ControllerMenuTests(unittest.TestCase):
    def test_select_press_release_rebind_and_optional_profiles(self):
        code = r'''
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#include "dm_input.h"
#include "DmControllerMenu.h"
#include <assert.h>
@interface TestButton : NSObject
@property(getter=isPressed) BOOL pressed;
@property NSInteger preferredSystemGestureState;
@property(copy) GCControllerButtonValueChangedHandler pressedChangedHandler;
- (void)press:(BOOL)value;
@end
@implementation TestButton
- (void)press:(BOOL)value { self.pressed=value; if(self.pressedChangedHandler)self.pressedChangedHandler((id)self,value,value); }
@end
@interface TestDevice : NSObject
@property id physicalInputProfile;
@end
@implementation TestDevice @end
@interface TestProfile : NSObject
@property id buttonOptions,buttonMenu;
@property TestDevice *controller;
@property NSDictionary *buttons;
@end
@implementation TestProfile @end
static TestProfile *profile(TestButton *menu, TestButton *options, NSDictionary *buttons) {
 TestProfile *p=[TestProfile new];p.buttonOptions=options;p.buttonMenu=menu;p.buttons=buttons;
 p.controller=[TestDevice new];p.controller.physicalInputProfile=p;return p;
}
int main(void){@autoreleasepool{
 unsigned checks=0;
 // Native Options beats aliases; two callbacks on the same state do not add bits.
 TestButton *plus=[TestButton new],*minus=[TestButton new],*share=[TestButton new];
 TestProfile *p=profile(plus,minus,@{@"Share":share});GCExtendedGamepad *g=(id)p;
 __block uint64_t state=0,latch=0;__block unsigned edges=0;
 DmControllerMenuChanged publish=^{uint64_t next=dm_controller_menu_bits(g);latch|=next&~state;if(next!=state)edges++;state=next;};
 dm_controller_bind_menu(g,nil,publish);assert(dm_controller_select(g)==(id)minus);assert(plus.preferredSystemGestureState==GCSystemGestureStateDisabled&&minus.preferredSystemGestureState==GCSystemGestureStateDisabled);checks+=2;
 [minus press:YES];publish();assert(state==DM_BTN_MINUS&&latch==DM_BTN_MINUS&&edges==1);[minus press:NO];assert(state==0&&edges==2);checks+=2;
 [plus press:YES];[minus press:YES];assert(state==(DM_BTN_PLUS|DM_BTN_MINUS));[plus press:NO];assert(state==DM_BTN_MINUS);[minus press:NO];assert(state==0);checks+=3;
 // Rebinding replaces old callbacks, does not stack them or reuse stale aliases.
 TestButton *replacement=[TestButton new];p.buttonOptions=replacement;dm_controller_bind_menu(g,nil,publish);assert(!minus.pressedChangedHandler);[replacement press:YES];assert(state==DM_BTN_MINUS);[replacement press:NO];assert(state==0);checks+=3;
 dm_controller_unbind_menu(g);assert(!plus.pressedChangedHandler&&!replacement.pressedChangedHandler);checks++;
 // Each supported explicit firmware alias, with standard Options nil.
 for(NSString *key in @[GCInputButtonOptions,@"Select",@"VIEW",@"Back",@"Minus",@"-",@"\u2212"]){
  TestButton *select=[TestButton new];TestProfile *q=profile(plus,nil,@{key:select});
  dm_controller_bind_menu((id)q,nil, ^{});assert(dm_controller_select((id)q)==(id)select);[select press:YES];assert(dm_controller_menu_bits((id)q)==DM_BTN_MINUS);[select press:NO];assert(dm_controller_menu_bits((id)q)==0);dm_controller_unbind_menu((id)q);checks+=3;
 }
 // Home/Share/face buttons are not guessed as Select. Nor is a Plus alias.
 for(NSDictionary *buttons in @[@{@"Home":share,@"Share":share,@"Button A":share},@{@"Select":plus},@{}]){
  TestProfile *q=profile(plus,nil,buttons);dm_controller_bind_menu((id)q,nil, ^{});assert(!dm_controller_select((id)q));assert(dm_controller_menu_bits((id)q)==0);dm_controller_unbind_menu((id)q);checks+=2;
 }
 TestProfile *same=profile(plus,plus,@{});dm_controller_bind_menu((id)same,nil, ^{});assert(!dm_controller_select((id)same));dm_controller_unbind_menu((id)same);checks++;
 // Separate controllers never clear or inherit each other's Select state.
 TestButton *one=[TestButton new],*two=[TestButton new];TestProfile *a=profile(plus,one,@{}),*b=profile([TestButton new],two,@{});
 dm_controller_bind_menu((id)a,nil, ^{});dm_controller_bind_menu((id)b,nil, ^{});[one press:YES];assert(dm_controller_menu_bits((id)a)==DM_BTN_MINUS&&dm_controller_menu_bits((id)b)==0);dm_controller_unbind_menu((id)a);[two press:YES];assert(dm_controller_menu_bits((id)b)==DM_BTN_MINUS);[two press:NO];assert(!dm_controller_menu_bits((id)b));dm_controller_unbind_menu((id)b);checks+=3;
 // Explicit compatibility choice may use DS4 touchpad/Share; Auto never guesses it.
 TestButton *touch=[TestButton new];TestProfile *manual=profile([TestButton new],[TestButton new],@{GCInputDualShockTouchpadButton:touch});
 dm_controller_bind_menu((id)manual,GCInputDualShockTouchpadButton, ^{});assert(dm_controller_select((id)manual)==(id)touch);[touch press:YES];assert(dm_controller_menu_bits((id)manual)==DM_BTN_MINUS);[touch press:NO];assert(dm_controller_menu_bits((id)manual)==0);dm_controller_unbind_menu((id)manual);checks+=3;
 dm_controller_bind_menu((id)manual,@"missing-source", ^{});assert(dm_controller_select((id)manual)==manual.buttonOptions);dm_controller_unbind_menu((id)manual);checks++;
 dm_controller_unbind_menu(nil);printf("%u controller menu assertions passed\n",checks);
}}
'''
        with tempfile.TemporaryDirectory() as tmp:
            d = pathlib.Path(tmp)
            (d / 'test.m').write_text(code)
            subprocess.run(['xcrun', 'clang', '-fobjc-arc', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'dm/ios/Sources'), '-I', str(ROOT / 'dm/host'),
                            str(d / 'test.m'), '-framework', 'Foundation', '-framework', 'GameController',
                            '-o', str(d / 'test')], check=True)
            subprocess.run([str(d / 'test')], check=True)

if __name__ == '__main__':
    unittest.main()
