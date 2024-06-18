#include <PDM.h>
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_BHY2Host.h"

SensorBSEC bsec(SENSOR_ID_BSEC);

Arduino_H7_Video Display(800, 480, GigaDisplayShield);

static void set_slider_val(void *bar, int32_t val) {
  lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_ON);
}

lv_anim_t a;
float iaqValue;

void setup() {
  Serial.begin(115200);
  // while (!Serial)
  //   ;

  BHY2Host.begin(false, NICLA_VIA_BLE);

  bsec.begin();

  Display.begin();

  /* Create a container with grid 2x2 */
  static lv_coord_t col_dsc[] = { 370, 370, LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[] = { 215, 215, LV_GRID_TEMPLATE_LAST };
  lv_obj_t *cont = lv_obj_create(lv_scr_act());
  lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
  lv_obj_set_size(cont, Display.width(), Display.height());
  lv_obj_set_style_bg_color(cont, lv_color_hex(0x03989e), LV_PART_MAIN);
  lv_obj_center(cont);

  lv_obj_t *label;
  lv_obj_t *obj;

  /* [0;0] - Image */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);

  LV_IMG_DECLARE(img_arduinologo);
  lv_obj_t *img1 = lv_img_create(obj);
  lv_img_set_src(img1, &img_arduinologo);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(img1, 200, 150);

  /* [1;1] - Bar */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_STRETCH, 1, 1);

  lv_obj_t *bar = lv_bar_create(obj);
  lv_obj_set_size(bar, 200, 20);
  lv_obj_center(bar);
  lv_bar_set_value(bar, 70, LV_ANIM_OFF);

  // Create the animation for the bar
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, set_slider_val);
  lv_anim_set_time(&a, 3000);
  lv_anim_set_playback_time(&a, 3000);
  lv_anim_set_var(&a, bar);

  delay(2000);
  Serial.println("Starting...");
}

void loop() {
  static auto printTime = millis();
  static float prevIaqValue;
  BHY2Host.update();

  if (millis() - printTime >= 5000) {
    printTime = millis();
    // Serial.println(String("IAQ: ") + bsec.toString());
    iaqValue = bsec.iaq();
    // prevIaqValue = iaqValue / 3;
    Serial.println(String("IAQ: ") + iaqValue);

    lv_anim_set_values(&a, 0, iaqValue / 3);
    lv_anim_start(&a);
  }

  lv_timer_handler();
}
