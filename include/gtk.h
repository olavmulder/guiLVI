#include <gtk/gtk.h>
#include "syncAlg.h"
#include "IF.h"
typedef enum _State
{
    Err = -1, RED, GREEN, YELLOW
}State;




bool CallbackDrowDown(gpointer data);
bool CallbackTemperature(gpointer data);
GtkWidget* InitDropDown(gint type);

void* gui(void*name);