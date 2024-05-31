#pragma once

#include <QtGui>
#include <QFrame>

class CMeter : public QFrame
{
    Q_OBJECT

public:
    explicit CMeter(QWidget *parent = 0);
    ~CMeter();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setLevel(float dbfs);
    void setSqlLevel(float dbfs);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void draw(QPainter &painter);
    void drawOverlay(QPainter &painter);

    float   m_dBFS;
    float   m_Sql;
};
