#ifndef FREQCTRL_H
#define FREQCTRL_H

#include <QtGui>
#include <QFrame>
#include <QImage>

#define TRUE true
#define FALSE false

enum FUNITS{UNITS_HZ, UNITS_KHZ, UNITS_MHZ, UNITS_GHZ,UNITS_SEC,UNITS_MSEC,UNITS_USEC,UNITS_NSEC };

#define MAX_DIGITS 12
#define MIN_DIGITS 4

class CFreqCtrl : public QFrame
{
    Q_OBJECT
public:
    explicit CFreqCtrl(QWidget *parent = 0);
    ~CFreqCtrl();
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    //primary access routines
    void Setup(int NumDigits, qint64 Minf, qint64 Maxf,int MinStep, FUNITS UnitsType);
    void SetFrequency(qint64 freq, bool emitSignal = true);
    void SetUnits(FUNITS units);
    void SetDigitColor(QColor cr);
    void SetBkColor(QColor cr);
    void SetUnitsColor(QColor cr);
    void SetHighlightColor(QColor cr);
    qint64 GetFrequency(){return m_freq;}

    void setG_constant(const qreal &value);

signals:
    void NewFrequency(qint64 freq); //emitted when frequency has changed

public slots:

protected:      //overrides for this control
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent* );
    void mouseMoveEvent(QMouseEvent * );
    void mousePressEvent(QMouseEvent * );
    void wheelEvent( QWheelEvent *  );
    void leaveEvent( QEvent *  );
    void keyPressEvent( QKeyEvent *  );

private:
    void UpdateCtrl(bool all);
    void DrawBkGround(QPainter &Painter);
    void DrawDigits(QPainter &Painter);
    void IncDigit();
    void DecDigit();
    void IncFreq();
    void DecFreq();
    void CursorHome();
    void CursorEnd();
    void MoveCursorLeft();
    void MoveCursorRight();
    bool InRect(QRect &rect, const QPoint &point);

    bool m_UpdateAll;
    bool m_ExternalKeyActive;
    bool m_LRMouseFreqSel;    /*! Use left/right mouse buttons. If FALSE click area determines up/down. */

    bool m_ResetLowerDigits;  /*! If TRUE digits below the active one will be reset to 0
                                  when the active digit is incremented or decremented. */

    int m_FirstEditableDigit;
    int m_LastLeadZeroPos;
    int m_LeadZeroPos;
    int m_NumDigits;
    int m_DigStart;
    int m_ActiveEditDigit;
    int m_LastEditDigit;
    int m_DecPos;
    int m_NumSeps;

    qint64 m_MinStep;
    qint64 m_freq;
    qint64 m_Oldfreq;
    qint64 m_MinFreq;
    qint64 m_MaxFreq;

    qreal g_constant;

    QColor m_DigitColor;
    QColor m_BkColor;
    QColor m_UnitsColor;
    QColor m_HighlightColor;

    QPixmap m_Pixmap;
    QSize m_Size;
    FUNITS m_Units;

    QRect m_rectCtrl;               //main control rectangle
    QRect m_UnitsRect;              //rectangle where Units text goes
    QRect m_SepRect[MAX_DIGITS];    //separation rectangles for commas,dec pt, etc.

    QString m_UnitString;

    QFont m_DigitFont;
    QFont m_UnitsFont;

    struct DigStuct
    {
        qint64 weight;  //decimal weight of this digit
        qint64 incval;  //value this digit increments or decrements
        QRect dQRect;   //Digit bounding rectangle
        int val;    //value of this digit(0-9)
        bool modified;  //set if this digit has been modified
        bool editmode;  //set if this digit is selected for editing
    }m_DigitInfo[MAX_DIGITS];
};

#endif // FREQCTRL_H
