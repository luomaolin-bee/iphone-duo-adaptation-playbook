# iOS controller Select compatibility example

Focused delta for the local nxnative Kirby host. This folder is not a complete game port and contains no game binary, shaders or keys. Preserve the existing host/player-slot integration and dm_input.h bit layout.

Copy DmControllerMenu.h to dm/ios/Sources and test_controller_menu.py to dm/ios/tests in the matching local host. Review integration.patch against the user's v15 UI baseline before applying it; it is not an upstream HEAD patch. Run python3 -m unittest discover -s dm/ios/tests -p test_controller_menu.py and rebuild the unsigned iOS host. The Python test expects to run from that host directory structure.

Native Options takes precedence in Auto; only explicit Select aliases are searched when absent. A user may explicitly choose a DS4 touchpad/Share/Home source that the current controller actually exposes. Sources are cached at bind time, and duplicate callback delivery publishes state rather than synthesizing another pulse. The test covers 46 assertions; real Pocket Taco firmware/system behavior remains pending.

The integration patch includes context from seedds/nxnative (Copyright 2026 seedds, MIT); see UPSTREAM-LICENSE.txt. This upstream license is preserved separately from the owner's original-document policy. New first-party helper/test code follows the owner's original-code policy in ../../LICENSE.md; the preserved upstream MIT terms apply to upstream patch context and are not replaced by that policy.

[Chinese issue/evidence record](../../docs/cases/kirby-select-v16.md).
