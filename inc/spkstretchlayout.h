
#pragma once

#include <QtWidgets>
#include <QVector>

/**
 * @brief SpkStretchLayout is meant solely for use with app list, where each layout item is
 *        considered the same size.
 */
class SpkStretchLayout : public QLayout
{
  public:
    SpkStretchLayout(QWidget *parent = nullptr);
    ~SpkStretchLayout();

    void addItem(QLayoutItem *item) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    int count() const override;
    QLayoutItem* itemAt(int) const override;
    QLayoutItem* takeAt(int) override;
    void setGeometry(const QRect &rect) override;

  private:
    QVector<QLayoutItem*> mItems;
};
