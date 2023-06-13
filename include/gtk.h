#include <gtk/gtk.h>
#include "syncAlg.h"
#include "IF.h"





bool CallbackDrowDown(gpointer data);
bool CallbackTemperature(gpointer data);
GtkWidget* InitDropDown(gint type);

void* gui(void*name);