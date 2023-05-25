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
/*
static void angle_changed (GtkGestureRotate *gesture,
               double            angle,
               double            delta)
{
  CanvasItem *item = CANVAS_ITEM (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture)));

  item->delta = angle / M_PI * 180.0;

  apply_transform (item);
}
*/
/*
static void rotate_done (GtkGesture *gesture)
{
  CanvasItem *item = CANVAS_ITEM (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture)));

  item->angle = item->angle + item->delta;
  item->delta = 0;
}
*/
static void click_done (GtkGesture *gesture)
{
  GtkWidget *item = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
  GtkWidget *canvas = gtk_widget_get_parent (item);
  GtkWidget *last_child;

  last_child = gtk_widget_get_last_child (canvas);
  if (item != last_child)
    gtk_widget_insert_after (item, canvas, last_child);
}
/*
static gboolean theme_is_dark (void)
{
  GtkSettings *settings;
  char *theme;
  gboolean prefer_dark;
  gboolean dark;

  settings = gtk_settings_get_default ();
  g_object_get (settings,
                "gtk-theme-name", &theme,
                "gtk-application-prefer-dark-theme", &prefer_dark,
                NULL);

  if ((strcmp (theme, "Adwaita") == 0 && prefer_dark) || strcmp (theme, "HighContrastInverse") == 0)
    dark = TRUE;
  else
    dark = FALSE;

  g_free (theme);

  return dark;
}*/

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


  /*if (theme_is_dark ())
    gdk_rgba_parse (&rgba, "blue");
  else
    gdk_rgba_parse (&rgba, "yellow");*/

  //TODO dropdown?
  
  gdk_rgba_parse (&rgba, colorLVI[0]);
  set_color (it, &rgba);

  it->angle = 0;

  dest = gtk_drop_target_new (G_TYPE_INVALID, GDK_ACTION_COPY);
  gtk_drop_target_set_gtypes (dest, types, G_N_ELEMENTS (types));
  g_signal_connect (dest, "drop", G_CALLBACK (item_drag_drop), NULL);
  gtk_widget_add_controller (GTK_WIDGET (it->label), GTK_EVENT_CONTROLLER (dest));

  /*gesture = gtk_gesture_rotate_new ();
  g_signal_connect (gesture, "angle-changed", G_CALLBACK (angle_changed), NULL);
  g_signal_connect (gesture, "end", G_CALLBACK (rotate_done), NULL);
  gtk_widget_add_controller (GTK_WIDGET (item), GTK_EVENT_CONTROLLER (gesture));*/

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

//should not be possible
/*
static void text_changed (GtkEditable *editable,
              GParamSpec  *pspec,
              CanvasItem  *item)
{
  gtk_label_set_text (GTK_LABEL (item->label), gtk_editable_get_text (editable));
  apply_transform (item);
}*/
/*
static void canvas_item_stop_editing (CanvasItem *item)
{
  GtkWidget *scale;

  if (!item->editor)
    return;

  scale = gtk_widget_get_last_child (item->editor);
  g_signal_handlers_disconnect_by_func (scale, scale_changed, item);

  gtk_fixed_remove (GTK_FIXED (gtk_widget_get_parent (item->editor)), item->editor);
  item->editor = NULL;
}

static void canvas_item_start_editing (CanvasItem *item)
{
  GtkWidget *canvas = gtk_widget_get_parent (GTK_WIDGET (item));
  GtkWidget *entry;
  GtkWidget *scale;
  graphene_point_t p;

  if (item->editor)
    return;

  item->editor = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

  entry = gtk_entry_new ();

  gtk_editable_set_text (GTK_EDITABLE (entry),
                         gtk_label_get_text (GTK_LABEL (item->label)));

  gtk_editable_set_width_chars (GTK_EDITABLE (entry), 12);
  g_signal_connect (entry, "notify::text", G_CALLBACK (text_changed), item);
  g_signal_connect_swapped (entry, "activate", G_CALLBACK (canvas_item_stop_editing), item);

  gtk_box_append (GTK_BOX (item->editor), entry);

  scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 360, 1);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_range_set_value (GTK_RANGE (scale), fmod (item->angle, 360));

  g_signal_connect (scale, "value-changed", G_CALLBACK (scale_changed), item);

  gtk_box_append (GTK_BOX (item->editor), scale);

  if (!gtk_widget_compute_point (GTK_WIDGET (item), canvas, &GRAPHENE_POINT_INIT (0, 0), &p))
    graphene_point_init (&p, 0, 0);
  gtk_fixed_put (GTK_FIXED (canvas), item->editor, p.x, p.y + 2 * item->r);
  gtk_widget_grab_focus (entry);

}
*/
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

 // if (!canvas_item_is_editing (item))
   // canvas_item_start_editing (item);
}

static void delete_cb (GtkWidget *button, GtkWidget *child)
{
  GtkWidget *canvas = gtk_widget_get_parent (child);

  gtk_fixed_remove (GTK_FIXED (canvas), child);

  gtk_popover_popdown (GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER)));
}
/*
static void pressed_cb (GtkGesture *gesture,
            int         n_press,
            double      x,
            double      y,
            gpointer    data)
{
  GtkWidget *widget;
  GtkWidget *child;
  //DROPDOWN menu

  widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
  child = gtk_widget_pick (widget, x, y, GTK_PICK_DEFAULT);
  child = gtk_widget_get_ancestor (child, canvas_item_get_type ());

  if (gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)) == GDK_BUTTON_SECONDARY)
    {
      GtkWidget *menu;
      GtkWidget *box;
      GtkWidget *item;

      menu = gtk_popover_new ();
      gtk_widget_set_parent (menu, widget);
      gtk_popover_set_has_arrow (GTK_POPOVER (menu), FALSE);
      gtk_popover_set_pointing_to (GTK_POPOVER (menu), &(GdkRectangle){ x, y, 1, 1});
      box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_popover_set_child (GTK_POPOVER (menu), box);

      item = gtk_button_new_with_label ("New");
      gtk_button_set_has_frame (GTK_BUTTON (item), FALSE);
      g_signal_connect (item, "clicked", G_CALLBACK (new_item_cb), widget);
      gtk_box_append (GTK_BOX (box), item);

      item = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
      gtk_box_append (GTK_BOX (box), item);

      item = gtk_button_new_with_label ("Edit");
      gtk_button_set_has_frame (GTK_BUTTON (item), FALSE);
      gtk_widget_set_sensitive (item, child != NULL && child != widget);
      g_signal_connect (item, "clicked", G_CALLBACK (edit_cb), child);
      gtk_box_append (GTK_BOX (box), item);

      item = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
      gtk_box_append (GTK_BOX (box), item);

      item = gtk_button_new_with_label ("Delete");
      gtk_button_set_has_frame (GTK_BUTTON (item), FALSE);
      gtk_widget_set_sensitive (item, child != NULL && child != widget);
      g_signal_connect (item, "clicked", G_CALLBACK (delete_cb), child);
      gtk_box_append (GTK_BOX (box), item);

      gtk_popover_popup (GTK_POPOVER (menu));
    }
    
}*/
/*
static void released_cb (GtkGesture *gesture,int n_press,
             double      x,
             double      y,
             gpointer    data)
{
  GtkWidget *widget;
  GtkWidget *child;
  CanvasItem *item;

  /*widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
  child = gtk_widget_pick (widget, x, y, 0);
  item = (CanvasItem *)gtk_widget_get_ancestor (child, canvas_item_get_type ());
  if (!item)
    return;

  if (gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)) == GDK_BUTTON_PRIMARY)
    {
      if (canvas_item_is_editing (item))
        canvas_item_stop_editing (item);
      else
        canvas_item_start_editing (item);
    }
    
}*/

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
  //g_signal_connect (gesture, "pressed", G_CALLBACK (pressed_cb), NULL);
  //g_signal_connect (gesture, "released", G_CALLBACK (released_cb), NULL);
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
/*
typedef struct
{
  GtkWidget parent_instance;
  GdkRGBA color;
} ColorSwatch;

typedef struct
{
  GtkWidgetClass parent_class;
} ColorSwatchClass;

G_DEFINE_TYPE (ColorSwatch, color_swatch, GTK_TYPE_WIDGET)

static GdkContentProvider *color_swatch_drag_prepare (GtkDragSource  *source,
                           double          x,
                           double          y,
                           ColorSwatch    *swatch)
{
  return gdk_content_provider_new_typed (GDK_TYPE_RGBA, &swatch->color);
}

static void color_swatch_init (ColorSwatch *swatch)
{
  GtkDragSource *source = gtk_drag_source_new ();
  g_signal_connect (source, "prepare", G_CALLBACK (color_swatch_drag_prepare), swatch);
  gtk_widget_add_controller (GTK_WIDGET (swatch), GTK_EVENT_CONTROLLER (source));
}

static void color_swatch_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  ColorSwatch *swatch = (ColorSwatch *)widget;
  float w = gtk_widget_get_width (widget);
  float h = gtk_widget_get_height (widget);

  gtk_snapshot_append_color (snapshot, &swatch->color,
                             &GRAPHENE_RECT_INIT(0, 0, w, h));
}

void color_swatch_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum_size,
                      int            *natural_size,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum_size = *natural_size = 48;
  else
    *minimum_size = *natural_size = 32;
}

static void color_swatch_class_init (ColorSwatchClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  widget_class->snapshot = color_swatch_snapshot;
  widget_class->measure = color_swatch_measure;
  gtk_widget_class_set_css_name (widget_class, "colorswatch");
}

static GtkWidget *color_swatch_new (const char *color)
{
  ColorSwatch *swatch = g_object_new (color_swatch_get_type (), NULL);

  gdk_rgba_parse (&swatch->color, color);

  return GTK_WIDGET (swatch);
}
*/

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
  
  if(firstTimeCallBack[id] == false)firstTimeCallBack[id]=true;
  
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
  
  return G_SOURCE_CONTINUE;

}
static GtkWidget *window = NULL;


static void do_dnd (GtkApplication *app, gpointer user_data)
{
  if (!window)
    {
      //window = gtk_window_new ();
      window = gtk_application_window_new (app);
     
      gtk_window_set_title (GTK_WINDOW (window), "Drag-and-Drop");
      gtk_window_set_default_size (GTK_WINDOW (window), 400,400);//1920, 1080);
      g_object_add_weak_pointer (G_OBJECT (window), (gpointer *)&window);
      GtkWidget *button;
      GtkWidget *sw;
      GtkWidget *canvas;
      GtkWidget *box, *box2;
      
      int i;
      int x, y;
      GtkCssProvider *provider;
      GString *css;

/*
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      button = gtk_color_button_new ();
G_GNUC_END_IGNORE_DEPRECATIONS
      g_object_unref (g_object_ref_sink (button));
*/const char *colors[] = {
        "red", "green", "blue", "magenta", "orange", "gray", "black", "yellow",
        "white", "gray", "brown", "pink",  "cyan", "bisque", "gold", "maroon",
        "navy", "orchid", "olive", "peru", "salmon", "silver", "wheat",
        NULL
      };
     

      css = g_string_new ("");
      for (i = 0; colors[i]; i++)
        g_string_append_printf (css, ".canvasitem.%s { background: %s; }\n", colors[i], colors[i]);
      
      g_string_append_printf(css, "window {background-image: url(\"img/E-H Deck1024_1.jpg\"); }");
      provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_data (provider, css->str, css->len);
      gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                  GTK_STYLE_PROVIDER (provider),
                                                  1080);
      //g_object_unref (provider);
      //g_string_free (css, TRUE);

      

      box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_window_set_child (GTK_WINDOW (window), box);

      box2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_box_append (GTK_BOX (box), box2);

      canvas = canvas_new ();
      gtk_box_append (GTK_BOX (box2), canvas);

      n_items = 0;

      x = y = 40;
      //make new item 
      for (i = 0; i < AMOUNT_NODES; i++)
      {
          item[i] = canvas_item_new ();
          gtk_fixed_put (GTK_FIXED (canvas), item[i], x, y);
          apply_transform (CANVAS_ITEM (item[i]));
          x += 75;
      }
      //canvasInitDone = true;
      //inti callback filtered temperature
      g_timeout_add(5000,G_SOURCE_FUNC(CallbackFilteredTemperature), NULL);

      gtk_box_append (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));

      /*sw = gtk_scrolled_window_new ();
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                      GTK_POLICY_AUTOMATIC,
                                      GTK_POLICY_NEVER);
      gtk_box_append (GTK_BOX (box), sw);*/

     
    }

  if (!gtk_widget_get_visible (window))
    gtk_widget_set_visible (window, TRUE);
  else
    gtk_window_destroy (GTK_WINDOW (window));
  //gtk_widget_show(window);
  //return window;
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
