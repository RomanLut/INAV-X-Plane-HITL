#pragma once

#include "config.h"

#include <functional>
#include <map>

typedef void (*WidgetValueChanged)(std::string value);

//=======================================================
//=======================================================
class TIPInputWidget
{
private:
  XPWidgetID widget = NULL;
  XPWidgetID textField = NULL;
  XPWidgetID okButton = NULL;
  XPWidgetID cancelButton = NULL;
  bool shown = false;
  WidgetValueChanged valueChanged = NULL;

  static int widgetMessageStatic(XPWidgetMessage inMessage, XPWidgetID  inWidget, intptr_t inParam1, intptr_t  inParam2);
  int widgetMessage(XPWidgetMessage inMessage, XPWidgetID  inWidget, intptr_t inParam1, intptr_t  inParam2);
  void close();
public:
  
  void create(int x, int y, int width, int height, std::string desc, std::string caption);
  void show();
  void registerValueChangedCb(WidgetValueChanged callback);
  std::string getValue();
  void setValue(std::string value);
};

extern TIPInputWidget g_IPInputWidget;