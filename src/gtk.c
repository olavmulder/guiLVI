#include "../include/gtk.h"

static const char* colorLVI[] = {"black", "red", "green", "yellow"};

GtkWidget *item[AMOUNT_NODES];
bool firstTimeCallBack[AMOUNT_NODES] = {false};

volatile bool canvasInitDone = false;

//const char *closeStateBlack= "Error";

const char *closeStateRed = "rood";
const char *closeStateYellow = "geel";
const char *closeStateGreen = "groen";

const char *voltageFree = "spannings vrij";
const char *voltageNotFree = "spanning hoog";

G_DECLARE_FINAL_TYPE (CanvasItem, canvas_item, CANVAS, ITEM, GtkWidget)

struct _CanvasItem {
  GtkWidget parent;

  GtkWidget *fixed;
  GtkWidget *label;
  GtkWidget *temperature;
  GtkWidget *dropdownClose;
  GtkWidget *dropdownVoltage;

  State state;
  guint id;
  double r;
  double angle;
  double delta;

  GtkWidget *editor;

  GtkStyleProvider *provider;
  char *css_class;
};

struct _CanvasItemClass {
  GtkWidgetClass parent_class;
};

G_DEFINE_TYPE (CanvasItem, canvas_item, GTK_TYPE_WIDGET)

static int n_items = 0;

static void unstyle_item (CanvasItem *item)
{
  if (item->provider)
    {
      gtk_style_context_remove_provider_for_display (gtk_widget_get_display (item->label), item->provider);
      g_clear_object (&item->provider);
    }

  if (item->css_class)
    {
      gtk_widget_remove_css_class (item->label, item->css_class);
      g_clear_pointer (&item->css_class, g_free);
    }
}
/*use to set color depanding on color*/
static void set_color (CanvasItem *item,
           GdkRGBA    *color)
{
  char *css;
  char *str;
  GtkCssProvider *provider;
  const char *name;

  unstyle_item (item);

  str = gdk_rgba_to_string (color);
  name = gtk_widget_get_name (item->label);
  if(strcmp(str, "rgb(255,255,0)") != 0)
    css = g_strdup_printf ("#%s { background: %s;color: #ffffff; }", name, str);
  else 
    css = g_strdup_printf ("#%s { background: %s;color: #000000; }", name, str);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider, css, (gssize)strlen(css));
  gtk_style_context_add_provider_for_display (gtk_widget_get_display (item->label), GTK_STYLE_PROVIDER (provider), 700);
  item->provider = GTK_STYLE_PROVIDER (provider);

  g_free (str);
  g_free (css);
}

static void set_css (CanvasItem *item,
         const char *class)
{
  unstyle_item (item);

  gtk_widget_add_css_class (item->label, class);
  item->css_class = g_strdup (class);
}

static gboolean item_drag_drop (GtkDropTarget *dest,
                const GValue  *value,
                double         x,
                double         y)
{
  GtkWidget *label = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (dest));
  CanvasItem *item = CANVAS_ITEM (gtk_widget_get_parent (gtk_widget_get_parent (label)));

  if (G_VALUE_TYPE (value) == GDK_TYPE_RGBA)
    set_color (item, g_value_get_boxed (value));
  else if (G_VALUE_TYPE (value) == G_TYPE_STRING)
    set_css (item, g_value_get_string (value));

  return TRUE;
}

static void apply_transform (CanvasItem *item)
{
  GskTransform *transform;
  double x, y;

  x = gtk_widget_get_allocated_width (item->label) / 2.0;
  y = gtk_widget_get_allocated_height (item->label) / 2.0;
  item->r = sqrt (x*x + y*y);

  transform = gsk_transform_translate (NULL, &(graphene_point_t) { item->r, item->r });
  transform = gsk_transform_rotate (transform, item->angle + item->delta);
  transform = gsk_transform_translate (transform, &(graphene_point_t) { -x, -y });

  gtk_fixed_set_child_transform (GTK_FIXED (item->fixed), item->label, transform);
  gsk_transform_unref (transform);
}

static void click_done (GtkGesture *gesture)
{
  GtkWidget *item = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
  GtkWidget *canvas = gtk_widget_get_parent (item);
  GtkWidget *last_child;

  last_child = gtk_widget_get_last_child (canvas);
  if (item != last_child)
    gtk_widget_insert_after (item, canvas, last_child);
}

//item item ding
static void canvas_item_init (CanvasItem *it)
{
  char *text;
  char *id;
  GdkRGBA rgba;
  GtkDropTarget *dest;
  GtkGesture *gesture;
  GType types[2] = { GDK_TYPE_RGBA, G_TYPE_STRING };

  
  //name of item
  text = g_strdup_printf ("lvi:%d", n_items);
  it->id = n_items;
  it->label = gtk_label_new (text);
  gtk_widget_add_css_class (it->label, "canvasitem");
  g_free (text);

  it->fixed = gtk_fixed_new ();
  gtk_widget_set_parent (it->fixed, GTK_WIDGET (it));
  gtk_fixed_put (GTK_FIXED (it->fixed), it->label, 0, 0);

  //gtk_widget_add_css_class (it->label, "frame");

  id = g_strdup_printf ("item%d", n_items);
  gtk_widget_set_name (it->label, id);
  g_free (id);

  //add close state dropdown to canvas item
  it->dropdownClose = InitDropDown(0);
  gtk_fixed_put (GTK_FIXED (it->fixed), it->dropdownClose, 0, 10);
  //add voltage state dropdown to canvas item
  it->dropdownVoltage = InitDropDown(1);
  gtk_fixed_put (GTK_FIXED (it->fixed), it->dropdownVoltage, 0, 45);
  g_timeout_add(100,G_SOURCE_FUNC(CallbackDrowDown), it);

  
  //add label for temperature
  text = g_strdup_printf ("°C");
  it->temperature = gtk_label_new (text);
  //gtk_widget_add_css_class (it->temperature, "canvasitem");
  gtk_fixed_put (GTK_FIXED (it->fixed), it->temperature, 10, 80);
  g_timeout_add(500,G_SOURCE_FUNC(CallbackTemperature), it);


  
  gdk_rgba_parse (&rgba, colorLVI[0]);
  set_color (it, &rgba);

  it->angle = 0;

  dest = gtk_drop_target_new (G_TYPE_INVALID, GDK_ACTION_COPY);
  gtk_drop_target_set_gtypes (dest, types, G_N_ELEMENTS (types));
  g_signal_connect (dest, "drop", G_CALLBACK (item_drag_drop), NULL);
  gtk_widget_add_controller (GTK_WIDGET (it->label), GTK_EVENT_CONTROLLER (dest));

 
  gesture = gtk_gesture_click_new ();
  g_signal_connect (gesture, "released", G_CALLBACK (click_done), NULL);
  gtk_widget_add_controller (GTK_WIDGET (item), GTK_EVENT_CONTROLLER (gesture));
  n_items++;
}

static void canvas_item_dispose (GObject *object)
{
  CanvasItem *item = CANVAS_ITEM (object);

  g_clear_pointer (&item->fixed, gtk_widget_unparent);
  g_clear_pointer (&item->editor, gtk_widget_unparent);

  G_OBJECT_CLASS (canvas_item_parent_class)->dispose (object);
}

static void canvas_item_map (GtkWidget *widget)
{
  CanvasItem *item = CANVAS_ITEM (widget);

  GTK_WIDGET_CLASS (canvas_item_parent_class)->map (widget);

  apply_transform (item);
}

static void canvas_item_class_init (CanvasItemClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->dispose = canvas_item_dispose;

  widget_class->map = canvas_item_map;

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "item");
}

static GtkWidget *canvas_item_new (void)
{
  CanvasItem *item = g_object_new (canvas_item_get_type (), NULL);

  return GTK_WIDGET (item);
}

static GdkPaintable *canvas_item_get_drag_icon (CanvasItem *item)
{
  return gtk_widget_paintable_new (item->fixed);
}

static gboolean canvas_item_is_editing (CanvasItem *item)
{
  return item->editor != NULL;
}

static void scale_changed (GtkRange   *range,
               CanvasItem *item)
{
  item->angle = gtk_range_get_value (range);
  apply_transform (item);
}

typedef struct {
  double x, y;
} Hotspot;

static GdkContentProvider *prepare (GtkDragSource *source,
         double         x,
         double         y)
{
  GtkWidget *canvas;
  GtkWidget *item;
  Hotspot *hotspot;
  graphene_point_t p;

  canvas = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (source));
  item = gtk_widget_pick (canvas, x, y, GTK_PICK_DEFAULT);

  item = gtk_widget_get_ancestor (item, canvas_item_get_type ());
  if (!item)
    return NULL;

  g_object_set_data (G_OBJECT (canvas), "dragged-item", item);

  hotspot = g_new (Hotspot, 1);
  if (!gtk_widget_compute_point (canvas, item, &GRAPHENE_POINT_INIT (x, y), &p))
    graphene_point_init (&p, x, y);
  hotspot->x = p.x;
  hotspot->y = p.y;
  g_object_set_data_full (G_OBJECT (canvas), "hotspot", hotspot, g_free);

  return gdk_content_provider_new_typed (GTK_TYPE_WIDGET, item);
}

static void drag_begin (GtkDragSource *source,
            GdkDrag       *drag)
{
  GtkWidget *canvas;
  CanvasItem *item;
  GdkPaintable *paintable;
  Hotspot *hotspot;

  canvas = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (source));
  item = CANVAS_ITEM (g_object_get_data (G_OBJECT (canvas), "dragged-item"));
  hotspot = (Hotspot *) g_object_get_data (G_OBJECT (canvas), "hotspot");

  paintable = canvas_item_get_drag_icon (item);
  gtk_drag_source_set_icon (source, paintable, hotspot->x, hotspot->y);
  g_object_unref (paintable);

  gtk_widget_set_opacity (GTK_WIDGET (item), 0.3);
}

static void drag_end (GtkDragSource *source,
          GdkDrag       *drag)
{
  GtkWidget *canvas;
  GtkWidget *item;

  canvas = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (source));
  item = g_object_get_data (G_OBJECT (canvas), "dragged-item");
  g_object_set_data (G_OBJECT (canvas), "dragged-item", NULL);

  gtk_widget_set_opacity (item, 1.0);
}

static gboolean
drag_cancel (GtkDragSource       *source,
             GdkDrag             *drag,
             GdkDragCancelReason  reason)
{
  return FALSE;
}

static gboolean drag_drop (GtkDropTarget *target,
           const GValue  *value,
           double         x,
           double         y)
{
  CanvasItem *item;
  GtkWidget *canvas;
  GtkWidget *last_child;

  item = g_value_get_object (value);

  canvas = gtk_widget_get_parent (GTK_WIDGET (item));
  last_child = gtk_widget_get_last_child (canvas);
  if (GTK_WIDGET (item) != last_child)
    gtk_widget_insert_after (GTK_WIDGET (item), canvas, last_child);

  gtk_fixed_move (GTK_FIXED (canvas), GTK_WIDGET (item), x - item->r, y - item->r);

  return TRUE;
}

static void new_item_cb (GtkWidget *button, gpointer data)
{
  GtkWidget *canvas = GTK_WIDGET(data);
  GtkWidget *popover;
  GtkWidget *item;
  GdkRectangle rect;

  popover = gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER);
  gtk_popover_get_pointing_to (GTK_POPOVER (popover), &rect);

  item = canvas_item_new ();
  gtk_fixed_put (GTK_FIXED (canvas), item, rect.x, rect.y);
  apply_transform (CANVAS_ITEM (item));

  gtk_popover_popdown (GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER)));
}

static void edit_cb (GtkWidget *button, GtkWidget *child)
{
  CanvasItem *item = CANVAS_ITEM (child);

  if (button)
    gtk_popover_popdown (GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER)));
}

static void delete_cb (GtkWidget *button, GtkWidget *child)
{
  GtkWidget *canvas = gtk_widget_get_parent (child);

  gtk_fixed_remove (GTK_FIXED (canvas), child);

  gtk_popover_popdown (GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER)));
}

static GtkWidget *canvas_new (void)
{
  GtkWidget *canvas;
  GtkDragSource *source;
  GtkDropTarget *dest;
  GtkGesture *gesture;

  canvas = gtk_fixed_new ();
  gtk_widget_set_hexpand (canvas, TRUE);
  gtk_widget_set_vexpand (canvas, TRUE);

  source = gtk_drag_source_new ();
  gtk_drag_source_set_actions (source, GDK_ACTION_MOVE);
  g_signal_connect (source, "prepare", G_CALLBACK (prepare), NULL);
  g_signal_connect (source, "drag-begin", G_CALLBACK (drag_begin), NULL);
  g_signal_connect (source, "drag-end", G_CALLBACK (drag_end), NULL);
  g_signal_connect (source, "drag-cancel", G_CALLBACK (drag_cancel), NULL);
  gtk_widget_add_controller (canvas, GTK_EVENT_CONTROLLER (source));

  dest = gtk_drop_target_new (GTK_TYPE_WIDGET, GDK_ACTION_MOVE);
  g_signal_connect (dest, "drop", G_CALLBACK (drag_drop), NULL);
  gtk_widget_add_controller (canvas, GTK_EVENT_CONTROLLER (dest));

  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);

  gtk_widget_add_controller (canvas, GTK_EVENT_CONTROLLER (gesture));

  return canvas;
}

static GdkContentProvider *css_drag_prepare (GtkDragSource *source,
                  double         x,
                  double         y,
                  GtkWidget     *button)
{
  const char *class;
  GdkPaintable *paintable;

  class = (const char *)g_object_get_data (G_OBJECT (button), "css-class");

  paintable = gtk_widget_paintable_new (button);
  gtk_drag_source_set_icon (source, paintable, 0, 0);
  g_object_unref (paintable);

  return gdk_content_provider_new_typed (G_TYPE_STRING, class);
}

static GtkWidget *css_button_new (const char *class)
{
  GtkWidget *button;
  GtkDragSource *source;

  button = gtk_image_new ();
  gtk_widget_set_size_request (button, 48, 32);
  gtk_widget_add_css_class (button, class);
  g_object_set_data (G_OBJECT (button), "css-class", (gpointer)class);

  source = gtk_drag_source_new ();
  g_signal_connect (source, "prepare", G_CALLBACK (css_drag_prepare), button);
  gtk_widget_add_controller (button, GTK_EVENT_CONTROLLER (source));

  return button;
}

static void setup_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *lb = gtk_label_new (NULL);
  gtk_list_item_set_child (listitem, lb);
  /* Because gtk_list_item_set_child sunk the floating reference of lb, releasing (unref) isn't necessary for lb. */
}

static void bind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *lb = gtk_list_item_get_child (listitem);
  /* Strobj is owned by the instance. Caller mustn't change or destroy it. */
  GtkStringObject *strobj = gtk_list_item_get_item (listitem);
  /* The string returned by gtk_string_object_get_string is owned by the instance. */
  gtk_label_set_text (GTK_LABEL (lb), gtk_string_object_get_string (strobj));
}

static GtkStringList* _InitStringList(gint type)
{
  if(!type){
    const char* strings[] = {"None", closeStateRed, closeStateGreen, closeStateYellow, NULL};
    return gtk_string_list_new ((const char * const *) strings);
  }else{
    const char* strings[] = {"volt", "volt vrij", NULL};
    return gtk_string_list_new ((const char * const *) strings);
  }
}

static GtkWidget* InitListFactory(gint type)
{
  GtkStringList* list = _InitStringList(type);
  GtkSingleSelection *ss =  gtk_single_selection_new (G_LIST_MODEL (list));
  
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
  g_signal_connect (factory, "setup", G_CALLBACK (setup_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_cb), NULL);
 
  GtkWidget *lv = gtk_list_view_new (GTK_SELECTION_MODEL (ss), factory);
  return lv;
}

GtkWidget* InitDropDown(gint type)
{
  GtkWidget* dd  = gtk_drop_down_new(G_LIST_MODEL(_InitStringList(type)),NULL );
  return dd;
}

bool CallbackFilteredTemperature(gpointer data)
{
  //get all data from all nodes
  size_t readings = 1;
  double temp[2];
  for(guint i = 0;i< 2;i++)
  {
    temp[i] = 19+i;//list[i].temp;
  }
  double* filterData = IFalg(temp,readings, 2);
  double val=0;
  size_t i = 0;
  while(!isnan(filterData[i]) && i <= readings*ITER && filterData[i] != 0)
  {
    i++;
  }
  if(i > 0){
    val = filterData[i-1];
    //g_print("filtered data%.2f\n", val);
  }
  else
    g_print("error filter data\n");
    

  free(filterData);
  return G_SOURCE_CONTINUE;
}
bool CallbackTemperature(gpointer data)
{
  guint id = CANVAS_ITEM(data)->id;
  double temperature = list[id].temp;
  char str[20];
  snprintf(str, 20, "%.2f°C", temperature);
  gtk_label_set_text(GTK_LABEL(CANVAS_ITEM(data)->temperature), str);
  return G_SOURCE_CONTINUE;
}
//get periodically status from dropdown and set closeState in datalist  
//
bool CallbackDrowDown(gpointer data)
{

  bool changeVolt = false, changeClose = false;
  static unsigned int max_id = 0;
  static bool isSet = false;
  static int8_t *lastSelectValuesClose;
  static int8_t *lastSelectValuesVolt;

  unsigned int id = CANVAS_ITEM(data)->id;

  //make static dynamic allocted array
  if(max_id < id)
  {
    max_id = id;
  }
  if(!isSet)
  {
    lastSelectValuesClose = (int8_t*)malloc(sizeof(int8_t) * (max_id+1));
    lastSelectValuesVolt = (int8_t*)malloc(sizeof(int8_t) * (max_id+1));

    isSet = true;
  }
  else{
    lastSelectValuesClose = realloc(lastSelectValuesClose, sizeof(int8_t) * (max_id+1));
    lastSelectValuesVolt = realloc(lastSelectValuesVolt, sizeof(int8_t) * (max_id+1));
  }


  GdkRGBA color;

  int8_t tempClose = (int8_t)gtk_drop_down_get_selected(GTK_DROP_DOWN(CANVAS_ITEM(data)->dropdownClose))-1;  
  int8_t tempVolt = (int8_t)gtk_drop_down_get_selected(GTK_DROP_DOWN(CANVAS_ITEM(data)->dropdownVoltage));  
  
  //if the selected one is others than the last selected one
  //closeing state
  if(tempClose != lastSelectValuesClose[id])
  {
    lastSelectValuesClose[id] = tempClose;
    list[id].closeState = tempClose;
    gdk_rgba_parse(&color, colorLVI[tempClose+1]);//+1 because of black state at index 0 
    set_color(CANVAS_ITEM(data), &color);
    changeClose = true;
  }
  //closestate has changed through sync or send from a device, so change select value
  else if(tempClose != list[id].closeState)
  {
    if(list[id].closeState >= -1 && list[id].closeState <= 2)
    {
      tempClose = list[id].closeState;
      gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(data)->dropdownClose),(unsigned int)tempClose+1);
      lastSelectValuesClose[id] = list[id].closeState;
      gdk_rgba_parse(&color, colorLVI[tempClose+1]);
      set_color(CANVAS_ITEM(data), &color);
    }
  }

  //voltage state
  //if the selected one is others than the last selected one
  if(tempVolt != lastSelectValuesVolt[id])
  {
    lastSelectValuesVolt[id] = tempVolt;
    list[id].voltageState = tempVolt;
    changeVolt = true;

  }
  //voltage state has changed through sync, so change select value
  else if(tempVolt != list[id].voltageState)
  {
    
    if(list[id].voltageState >= 0 && list[id].voltageState <= 1)
    {
      tempVolt = list[id].voltageState;
      lastSelectValuesVolt[id] = list[id].voltageState;
      gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(data)->dropdownVoltage),(unsigned int)tempVolt);
    }
  }
  
  if(firstTimeCallBack[id] == false)
    firstTimeCallBack[id]=true;
  else
  {
    if(changeClose && changeVolt)
    {
      printf("A\n");
      MakeChangeLog(id,tempVolt,tempClose,0,false, true, true, false, false);
    }
    else if(changeClose && !changeVolt)
    {
      printf("B\n");
      MakeChangeLog(id,0,tempClose,0,false, false, true, false, false);
    }
    else if(!changeClose && changeVolt)
    {
      printf("C\n");
      MakeChangeLog(id,tempVolt,0,0,false, true, false, false, false);
    }
  }
  if(!canvasInitDone)
  {
    for(uint8_t i = 0 ; i < AMOUNT_NODES; i++)
    {
      if(firstTimeCallBack[id] == false)
        return G_SOURCE_CONTINUE;
    }
    canvasInitDone = true;
    g_print("canvas init done\n");
  }
  
  return G_SOURCE_CONTINUE;

}

void ButtonCallback(GtkToggleButton *button, gpointer user_data)
{
  gboolean active = gtk_toggle_button_get_active(button);
  switch((int)user_data)
  {
    case 0:
      if(active)
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[1])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[2])->dropdownClose),(unsigned int)2+1);

      }
      else
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[1])->dropdownClose),(unsigned int)1+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[2])->dropdownClose),(unsigned int)1+1);

      }
      break;
    case 1:
      if(active)
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[1])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[2])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[3])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[4])->dropdownClose),(unsigned int)2+1);

      }
      else
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[1])->dropdownClose),(unsigned int)1+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[2])->dropdownClose),(unsigned int)1+1);

      }
      break;
    case 2:
      if(active)
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[1])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[2])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[3])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[4])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[5])->dropdownClose),(unsigned int)2+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[6])->dropdownClose),(unsigned int)2+1);

      }
      else
      {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[5])->dropdownClose),(unsigned int)1+1);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(CANVAS_ITEM(item[6])->dropdownClose),(unsigned int)1+1);
        
      }
      break;
    default:
      break;
  };

}
void XYZ_Button(GtkWidget** buttonX, GtkWidget** buttonY, GtkWidget** buttonZ)
{

  *buttonX = gtk_toggle_button_new_with_label("X");
  *buttonY = gtk_toggle_button_new_with_label("Y");
  *buttonZ = gtk_toggle_button_new_with_label("Z");
  g_signal_connect(*buttonX, "clicked", G_CALLBACK(ButtonCallback), (void*)0);
  g_signal_connect(*buttonY, "clicked", G_CALLBACK(ButtonCallback), (void*)1);
    g_signal_connect(*buttonZ, "clicked", G_CALLBACK(ButtonCallback), (void*)2);
  //Apply CSS styling to the button
  gtk_widget_add_css_class(*buttonX, "custom-button");
  gtk_widget_add_css_class(*buttonY, "custom-button");
  gtk_widget_add_css_class(*buttonZ, "custom-button");
  gtk_widget_set_size_request(*buttonX, 20, 20);
  gtk_widget_set_size_request(*buttonY, 20, 20);
  gtk_widget_set_size_request(*buttonZ, 20, 20);


}
static GtkWidget *window = NULL;


static void do_dnd (GtkApplication *app, gpointer user_data)
{
  if (!window)
    {
      //window = gtk_window_new ();
      window = gtk_application_window_new (app);
     
      gtk_window_set_title (GTK_WINDOW (window), "Centrale bedieningspaneel");
      gtk_window_set_default_size (GTK_WINDOW (window), 1920, 1080);
      g_object_add_weak_pointer (G_OBJECT (window), (gpointer *)&window);
      GtkWidget *button;
      GtkWidget *sw;
      GtkWidget *canvas;
      GtkWidget *box, *box2;
      GtkWidget *layout;
      GtkWidget *image;
      
      int i;
      int x, y;
      GtkCssProvider *provider;
      GString *css;

      const char *colors[] = {
        "red", "green", "blue", "magenta", "orange", "gray", "black", "yellow",
        "white", "gray", "brown", "pink",  "cyan", "bisque", "gold", "maroon",
        "navy", "orchid", "olive", "peru", "salmon", "silver", "wheat",
        NULL
      };
     

      css = g_string_new ("");
      for (i = 0; colors[i]; i++)
        g_string_append_printf (css, ".canvasitem.%s { background: %s; }\n", colors[i], colors[i]);
                              
      provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_data (provider, css->str, css->len);
      GtkCssProvider *cssProvider = gtk_css_provider_new();
      gtk_css_provider_load_from_path(cssProvider, "style.css"); // Replace with the path to your CSS file
      gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
      gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER (provider),  GTK_STYLE_PROVIDER_PRIORITY_USER);
      
      //box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      //box2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);


      canvas = canvas_new ();
      n_items = 0;

      y = 220;
      x = 1200;
      //make new item 
      for (i = 0; i < AMOUNT_NODES; i++)
      {
          item[i] = canvas_item_new ();
          gtk_fixed_put (GTK_FIXED (canvas), item[i], x, y);
          apply_transform (CANVAS_ITEM (item[i]));

            if( i % 2 == 0 && i != 0)
            {
              y += 150;
              x -= 75;
            }
            else 
              x += 75;
      }
      GtkWidget *overlay = gtk_overlay_new();
      GtkWidget *box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
      // Load the background image
      image = gtk_image_new_from_file("img/plat.jpg");  // Replace with the actual image path
      // Set the image as the overlay's background

      //add x,y,z status button
      GtkWidget* buttonX;
      GtkWidget* buttonY;
      GtkWidget* buttonZ;
      GtkWidget* box_button = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
      XYZ_Button(&buttonX, &buttonY, &buttonZ);
      gtk_box_append(GTK_BOX(box_button), buttonX);
      gtk_box_append(GTK_BOX(box_button), buttonY);
      gtk_box_append(GTK_BOX(box_button), buttonZ);
      
      gtk_box_append(GTK_BOX(box_main), canvas);
      gtk_box_append(GTK_BOX(box_main), box_button);

      
      gtk_overlay_set_child(GTK_OVERLAY(overlay), image);
      gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box_main );

      gtk_window_set_child (GTK_WINDOW (window), overlay);

      //inti callback filtered temperature
      g_timeout_add(5000,G_SOURCE_FUNC(CallbackFilteredTemperature), NULL);
      
     
      //gtk_box_append (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));
    }

  if (!gtk_widget_get_visible (window))
    gtk_widget_set_visible (window, TRUE);
  else
    gtk_window_destroy (GTK_WINDOW (window));

}


void *gui(void* name)
{
  GtkApplication *app;
  
  app = gtk_application_new ((char*)name, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (do_dnd), NULL);
  g_application_run (G_APPLICATION (app), 0, 0);
  g_object_unref (app);
  return NULL;
}
