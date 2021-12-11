
#include "spkpopup.h"
#include <QDebug>

namespace SpkUi
{
  SpkPopup::SpkPopup(QWidget *parent,  int aMillis) : QWidget(parent)
  {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    mText = new QLabel();
    mText->setStyleSheet("border-radius: 11px;"
                         "background-color: rgba(0,0,0,150);"
                         "color: white;"
                         "padding: 5px;");
    mText->setText(tr("(No Text)"));
    mBox = new QHBoxLayout(this);
    mBox->addWidget(mText);
    mBox->setContentsMargins(0, 0, 0, 0);
    // The reason why we contain it in a widget is that, if we want a QLabel have rounded corners,
    // then it must be able to be displayed with a translucent background. However, setting
    // Qt::WA_TranslucentBackground will cause the entire background of QLabel transparent.
    // Therefore we need a container (SpkPopup) with a transparent background as the canvas layer
    // of the actual displayed text.
    mAnim = new QSequentialAnimationGroup(this);

    // Disabled as translucency doesn't work well on every platform
//    mAnimFadeIn = new QPropertyAnimation(this, "windowOpacity");
//    mAnimFadeOut = new QPropertyAnimation(this, "windowOpacity");
//    mAnimFadeIn->setStartValue(0.0);
//    mAnimFadeIn->setEndValue(1.0);
//    mAnimFadeOut->setStartValue(1.0);
//    mAnimFadeOut->setEndValue(0.0);
    // Using moving animation instead
    mAnimFadeIn = new QPropertyAnimation(this, "pos");
    mAnimFadeOut = new QPropertyAnimation(this, "pos");
    mAnimFadeIn->setDuration(250);
    mAnimFadeOut->setDuration(250);
    mAnimFadeIn->setEasingCurve(QEasingCurve::InQuad);
    mAnimFadeOut->setEasingCurve(QEasingCurve::InQuad);

    mAnim->addAnimation(mAnimFadeIn);
    mAnim->addPause(aMillis);
    mAnim->addAnimation(mAnimFadeOut);
    setVisible(false);

    connect(mAnim, &QAnimationGroup::stateChanged,
            [=](QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
    {
      qDebug() << "OldState" << oldState << "NewState" << newState;
    });
  }

  void SpkPopup::Show(QString aText)
  {
    if(mAnim->state() == QSequentialAnimationGroup::Running)
      mAnim->stop();
    QSize parentSize = parentWidget()->size();
    mText->setText(aText);
    adjustSize();
    move(QPoint((parentSize.width() - width()) / 2, parentSize.height() - height() - 30)/* +
         parentWidget()->pos()*/);
    setMaximumWidth(parentSize.width() - 200);
    setWindowOpacity(1);
    show();

    mAnimFadeIn->setStartValue(QPoint((parentSize.width() - width()) / 2,
                                      parentSize.height() + height()));
    mAnimFadeIn->setEndValue(QPoint((parentSize.width() - width()) / 2,
                                    parentSize.height() - height() - 80));
    mAnimFadeOut->setStartValue(QPoint((parentSize.width() - width()) / 2,
                                       parentSize.height() - height() - 80));
    mAnimFadeOut->setEndValue(QPoint((parentSize.width() - width()) / 2,
                                     parentSize.height() + height()));

    qDebug() << "Popup size " << size() << "position" << pos() << "parent size" << parentWidget()->size();
    mAnim->start();
  }
}
