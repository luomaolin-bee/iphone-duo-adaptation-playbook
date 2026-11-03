// Select/Minus input for iOS controllers. Resolve only explicit Select aliases;
// never silently reuse Home, Share, a trigger or a face button as Select.
#pragma once
#include "dm_input.h"
#import <GameController/GameController.h>
#import <objc/runtime.h>

typedef void (^DmControllerMenuChanged)(void);
static char dm_menu_bound_key, dm_menu_select_key;
static inline GCControllerButtonInput *dm_controller_resolve_select(GCExtendedGamepad *g, NSString *source) {
    GCControllerButtonInput *chosen = source ? g.controller.physicalInputProfile.buttons[source] : nil;
    if (chosen && chosen != g.buttonMenu) return chosen;
    if (g.buttonOptions && g.buttonOptions != g.buttonMenu) return g.buttonOptions;
    NSDictionary<NSString *, GCControllerButtonInput *> *buttons = g.controller.physicalInputProfile.buttons;
    GCControllerButtonInput *options = buttons[GCInputButtonOptions];
    if (options && options != g.buttonMenu) return options;
    for (NSString *alias in @[@"select", @"view", @"back", @"minus", @"-", @"\u2212"]) {
        for (NSString *name in [[buttons allKeys] sortedArrayUsingSelector:@selector(compare:)]) {
            if ([name.lowercaseString isEqualToString:alias] && buttons[name] != g.buttonMenu) return buttons[name];
        }
    }
    return nil;
}
static inline GCControllerButtonInput *dm_controller_select(GCExtendedGamepad *g) {
    id cached = objc_getAssociatedObject(g, &dm_menu_select_key);
    return cached ? (cached == NSNull.null ? nil : cached) : dm_controller_resolve_select(g, nil);
}
static inline uint64_t dm_controller_menu_bits(GCExtendedGamepad *g) {
    return (g.buttonMenu.isPressed ? DM_BTN_PLUS : 0) | (dm_controller_select(g).isPressed ? DM_BTN_MINUS : 0);
}
static inline void dm_controller_unbind_menu(GCExtendedGamepad *g) {
    if (!g) return;
    for (GCControllerButtonInput *b in objc_getAssociatedObject(g, &dm_menu_bound_key)) b.pressedChangedHandler = nil;
    objc_setAssociatedObject(g, &dm_menu_bound_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(g, &dm_menu_select_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
static inline void dm_controller_bind_menu(GCExtendedGamepad *g, NSString *source, DmControllerMenuChanged changed) {
    dm_controller_unbind_menu(g);
    NSMutableArray<GCControllerButtonInput *> *bound = [NSMutableArray array];
    GCControllerButtonInput *select = dm_controller_resolve_select(g, source);
    objc_setAssociatedObject(g, &dm_menu_select_key, select ?: (id)NSNull.null, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    for (GCControllerButtonInput *b in @[g.buttonMenu ?: (id)NSNull.null, select ?: (id)NSNull.null]) {
        if ((id)b == NSNull.null || [bound containsObject:b]) continue;
        // Otherwise iOS may delay/consume menu events while recognizing a gesture.
        b.preferredSystemGestureState = GCSystemGestureStateDisabled;
        b.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            (void)button; (void)value; (void)pressed; changed();
        };
        [bound addObject:b];
    }
    objc_setAssociatedObject(g, &dm_menu_bound_key, bound, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
