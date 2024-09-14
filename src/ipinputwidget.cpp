#include "ipinputwidget.h"

TIPInputWidget g_IPInputWidget;

//=======================================================
//=======================================================
void TIPInputWidget::create(int x, int y, int width, int height, std::string desc, std::string caption)
{
  this->shown = false;
  int screenWidth = 0;
  int screenHeight = 0;
  XPLMGetScreenSize(&screenWidth, &screenHeight);

  int normY = screenHeight - y;

  this->widget = XPCreateWidget(x, normY, x + width, normY - height, 0, desc.c_str(), 1, NULL, xpWidgetClass_MainWindow);
  XPSetWidgetProperty(this->widget, xpProperty_MainWindowHasCloseBoxes, 1);

  XPCreateWidget(x + 10, normY - 30, x + 160, normY - 40, 1, caption.c_str(), 0, this->widget, xpWidgetClass_Caption);
  this->textField = XPCreateWidget(x + 10, normY - 50, x + 260, normY - 65, 1, "", 0, this->widget, xpWidgetClass_TextField);
  this->okButton = XPCreateWidget(x + 75, normY - 75, x + 125, normY - 95, 1, "OK", 0, this->widget, xpWidgetClass_Button);
  this->cancelButton = XPCreateWidget(x + 150, normY - 75, x + 200, normY - 95, 1, "Cancel", 0, this->widget, xpWidgetClass_Button);

  XPAddWidgetCallback(this->cancelButton, TIPInputWidget::widgetMessageStatic);
  XPAddWidgetCallback(this->okButton, TIPInputWidget::widgetMessageStatic);
  XPAddWidgetCallback(this->widget, TIPInputWidget::widgetMessageStatic);
}

//=======================================================
//=======================================================
void TIPInputWidget::show()
{
  if (!XPIsWidgetVisible(this->widget))
  {
    XPShowWidget(this->widget);
    this->shown = true;
  }
}

//=======================================================
//=======================================================
void TIPInputWidget::registerValueChangedCb(WidgetValueChanged callback)
{
  this->valueChanged = callback;
}

//=======================================================
//=======================================================
std::string TIPInputWidget::getValue()
{
  char value[100] = { 0 };
  XPGetWidgetDescriptor(this->textField, value, 100);
  return std::string(value);
}

//=======================================================
//=======================================================
void TIPInputWidget::setValue(std::string value)
{
  XPSetWidgetDescriptor(this->textField, value.c_str());
}

//=======================================================
//=======================================================
int TIPInputWidget::widgetMessage(XPWidgetMessage inMessage, XPWidgetID  inWidget, intptr_t inParam1, intptr_t  inParam2)
{
  if (this->shown)
  {
    if (((inWidget == this->widget) && (inMessage == xpMessage_CloseButtonPushed)) ||
      ((inWidget == this->cancelButton) && (inMessage == xpMsg_PushButtonPressed)))
    {
      close();
      return 1;
    }

    if (inWidget == this->okButton && inMessage == xpMsg_PushButtonPressed)
    {
      if (this->valueChanged != NULL)
      {
        this->valueChanged(this->getValue());
      }

      close();
      return 1;
    }
  }

  return 0;
}

//=======================================================
//=======================================================
void TIPInputWidget::close()
{
  XPHideWidget(this->widget);
  this->shown = false;
}

//=======================================================
//=======================================================
int TIPInputWidget::widgetMessageStatic(XPWidgetMessage inMessage, XPWidgetID  inWidget, intptr_t inParam1, intptr_t  inParam2)
{
  return g_IPInputWidget.widgetMessage(inMessage, inWidget, inParam1, inParam2);
}

