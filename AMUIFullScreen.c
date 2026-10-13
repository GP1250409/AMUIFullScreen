/* AMUIFullScreen: make the system "Automation Running" overlay fill the screen
 * on Plus (downsampled, 5.5") iPhones. Injected only into
 * com.apple.dt.AutomationModeUI.
 *
 * AutomationModeUI sizes its root overlay layer from CADisplay at a scale that
 * is wrong on Plus models (e.g. 360x640 on an 414x736 pt screen). We hook
 * CALayer -setBounds:/-setFrame: and, for a layer that is origin 0,0 and exactly
 * the wrong size, substitute the real point size = pixel size / native scale. */
typedef void *id;
extern id objc_getClass(const char *);
extern id sel_registerName(const char *);
extern id objc_msgSend(id, id, ...);
extern void *class_getInstanceMethod(id, id);
extern void *method_getImplementation(void *);
extern void *method_setImplementation(void *, void *);
typedef struct { double x, y, w, h; } Rect;
#define S(n) sel_registerName(n)

static double gW, gH;       /* real point size of the display */
static double gBadW, gBadH; /* size AutomationModeUI wrongly computed */
static double gPxW, gPxH;   /* render pixel size (1242x2208) */

static int is_bad(Rect r) {
  return gW > 0 && r.x == 0 && r.y == 0 &&
         r.w > gBadW - 0.5 && r.w < gBadW + 0.5 &&
         r.h > gBadH - 0.5 && r.h < gBadH + 0.5;
}

static void (*orig_setBounds)(id, id, Rect);
static void my_setBounds(id self, id sel, Rect r) {
  if (is_bad(r)) { r.w = gW; r.h = gH; }
  orig_setBounds(self, sel, r);
}
static void (*orig_setFrame)(id, id, Rect);
static void my_setFrame(id self, id sel, Rect r) {
  if (is_bad(r)) { r.w = gW; r.h = gH; }
  orig_setFrame(self, sel, r);
}

/* The root layer is also centred for the 1080x1920 panel: position (540,960)
 * with a 3x transform. Re-centre it for the real render size. */
static void (*orig_setPosition)(id, id, double, double);
static void my_setPosition(id self, id sel, double x, double y) {
  double bx = gPxW * 1080.0 / 1242.0 / 2.0, by = gPxH * 1080.0 / 1242.0 / 2.0;
  if (gPxW > 0 && x > bx - 0.5 && x < bx + 0.5 && y > by - 0.5 && y < by + 0.5) {
    x = gPxW / 2.0; y = gPxH / 2.0;
  }
  orig_setPosition(self, sel, x, y);
}

__attribute__((constructor)) static void init(void) {
  id disp = ((id (*)(id, id))objc_msgSend)(objc_getClass("CADisplay"), S("mainDisplay"));
  if (!disp) return;
  Rect b = ((Rect (*)(id, id))objc_msgSend)(disp, S("bounds"));   /* pixels */
  /* UIScreen's scale is not reachable from this process without UIKit init;
   * use the display's physical pixel size. Plus phones are 3x. */
  double scale = 3.0;
  gPxW = b.w; gPxH = b.h;
  gW = b.w / scale;
  gH = b.h / scale;
  /* Wrong size = physical panel pixels / 3: Plus panels are 1080x1920 while the
   * render size is 1242x2208 (observed: layer 360x640 on a 414x736 screen). */
  gBadW = gW * 1080.0 / 1242.0;
  gBadH = gH * 1080.0 / 1242.0;
  if (gW < 1 || gH < 1) return;

  id L = objc_getClass("CALayer");
  void *mp = class_getInstanceMethod(L, S("setPosition:"));
  if (mp) { orig_setPosition = (void *)method_getImplementation(mp); method_setImplementation(mp, (void *)my_setPosition); }
  void *m = class_getInstanceMethod(L, S("setBounds:"));
  if (m) { orig_setBounds = (void *)method_getImplementation(m); method_setImplementation(m, (void *)my_setBounds); }
  m = class_getInstanceMethod(L, S("setFrame:"));
  if (m) { orig_setFrame = (void *)method_getImplementation(m); method_setImplementation(m, (void *)my_setFrame); }
}
