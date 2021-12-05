#ifndef SPKICONBUTTON_H
#define SPKICONBUTTON_H

#include <QPushButton>
#include "spkui_general.h"

class SpkIconButton : public QPushButton
{
    Q_OBJECT

  public:
    SpkIconButton(QWidget *parent = nullptr);
    ~SpkIconButton() {};

    void SetIcon(QIcon, QSize);
    void SetIcon(QPixmap);
    void SetIconSize(QSize);

  protected:
    void paintEvent(QPaintEvent *) override;

  private:
    static enum { Dark, Light } sCurrentTheme;
    QPixmap mPmapPaintedIcon;
    QSize mPmapSize;

    static constexpr int IconMargin = 8; // shall we make it changable?
};

#endif // SPKICONBUTTON_H
