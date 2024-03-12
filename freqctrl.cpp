#include <QDebug>
#include "freqctrl.h"


//Manual adjustment of Font size as percent of control height
#define DIGIT_SIZE_PERCENT 50
#define UNITS_SIZE_PERCENT 30

//adjustment for separation between digits
#define SEPRATIO_N 100  //separation rectangle size ratio numerator times 100
#define SEPRATIO_D 3    //separation rectangle size ratio denominator

/////////////////////////////////////////////////////////////////////
// Constructor/Destructor
/////////////////////////////////////////////////////////////////////
CFreqCtrl::CFreqCtrl(QWidget *parent) :
    QFrame(parent)
{
    setAutoFillBackground(TRUE);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking ( TRUE );
    m_BkColor = QColor(0x20,0x20,0x20,0xFF);//Qt::black;
    m_DigitColor = QColor(0xFF, 0xE6, 0xC8, 0xFF);
    m_HighlightColor = QColor(0x5A, 0x5A, 0x5A, 0xFF);
    m_UnitsColor = Qt::gray;
    m_freq = 146123456;
    Setup( 10, 1, 4000000000U, 1, UNITS_MHZ);
    m_Oldfreq = 0;
    g_constant = 0;
    m_LastLeadZeroPos = 0;
    m_LRMouseFreqSel = FALSE;
    m_ActiveEditDigit = -1;
    m_ResetLowerDigits = FALSE;
    m_UnitsFont = QFont("Arial",14,QFont::Normal);
    m_DigitFont = QFont("Arial",14,QFont::Normal);
}

CFreqCtrl::~CFreqCtrl()
{
}

/////////////////////////////////////////////////////////////////////
//  Size hint stuff
/////////////////////////////////////////////////////////////////////
QSize CFreqCtrl::minimumSizeHint() const
{
    return QSize(100, 20);
}

QSize CFreqCtrl::sizeHint() const
{
    return QSize(100, 20);
}

/////////////////////////////////////////////////////////////////////
//  Various helper functions
/////////////////////////////////////////////////////////////////////
bool CFreqCtrl::InRect(QRect &rect, const QPoint &point)
{
    if( ( point.x() < rect.right( ) ) && ( point.x() > rect.x() ) &&
        ( point.y() < rect.bottom() ) && ( point.y() > rect.y() ) )
        return TRUE;
    else
        return FALSE;
}

void CFreqCtrl::setG_constant(const qreal &value)
{
    g_constant = value;
}

//////////////////////////////////////////////////////////////////////////////
//  Setup various parameters for the control
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::Setup(int NumDigits, qint64 Minf, qint64 Maxf,int MinStep, FUNITS UnitsType)
{
int i;
qint64 pwr = 1;
    m_LastEditDigit = 0;
    m_Oldfreq = -1;
    m_NumDigits = NumDigits;
    if( m_NumDigits>MAX_DIGITS )
        m_NumDigits = MAX_DIGITS;
    if( m_NumDigits<MIN_DIGITS )
        m_NumDigits = MIN_DIGITS;
    m_UnitString = "";
    m_MinStep = MinStep;
    if(m_MinStep==0)
        m_MinStep = 1;
    m_MinFreq = Minf;
    m_MaxFreq = Maxf;
    if( m_freq < m_MinFreq)
        m_freq = m_MinFreq;
    if( m_freq > m_MaxFreq)
        m_freq = m_MaxFreq;

    for(i=0; i<m_NumDigits; i++)
    {
        m_DigitInfo[i].weight = pwr;
        m_DigitInfo[i].incval = pwr;
        m_DigitInfo[i].modified = TRUE;
        m_DigitInfo[i].editmode = FALSE;
        m_DigitInfo[i].val = 0;
        pwr *= 10;
    }
    if( m_MaxFreq>pwr )
        m_MaxFreq = pwr-1;
    m_MaxFreq = m_MaxFreq - m_MaxFreq%m_MinStep;
    if( m_MinFreq>pwr )
        m_MinFreq = 1;
    m_MinFreq = m_MinFreq - m_MinFreq%m_MinStep;
    m_DigStart = 0;
    switch(UnitsType)
    {
        case UNITS_HZ:
            m_DecPos = 0;
            m_UnitString = "Hz ";
            break;
        case UNITS_KHZ:
            m_DecPos = 3;
            m_UnitString = "KHz";
            break;
        case UNITS_MHZ:
            m_DecPos = 6;
            m_UnitString = "MHz";
            break;
        case UNITS_GHZ:
            m_DecPos = 9;
            m_UnitString = "GHz";
            break;
        case UNITS_SEC:
            m_DecPos = 6;
            m_UnitString = "Sec";
            break;
        case UNITS_MSEC:
            m_DecPos = 3;
            m_UnitString = "mS ";
            break;
        case UNITS_USEC:
            m_DecPos = 0;
            m_UnitString = "uS ";
            break;
        case UNITS_NSEC:
            m_DecPos = 0;
            m_UnitString = "nS ";
            break;
    }
    for(i=m_NumDigits-1; i>=0; i--)
    {
        if( m_DigitInfo[i].weight <= m_MinStep )
        {
            if(m_DigStart == 0)
            {
                m_DigitInfo[i].incval = m_MinStep;
                m_DigStart = i;
            }
            else
            {
                if( (m_MinStep%m_DigitInfo[i+1].weight) != 0)
                    m_DigStart = i;
                m_DigitInfo[i].incval = 0;
            }
        }
    }
    m_NumSeps = (m_NumDigits-1)/3 - m_DigStart/3;
}

//////////////////////////////////////////////////////////////////////////////
//  Sets the frequency and individual digit values
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetFrequency(qint64 freq, bool emitSignal)
{
int i;
qint64 acc = 0;
qint64 rem;
int val;
    if( freq == m_Oldfreq)
        return;
    if( freq < m_MinFreq)
        freq = m_MinFreq;
    if( freq > m_MaxFreq)
        freq = m_MaxFreq;
    m_freq = freq - freq%m_MinStep;
    rem = m_freq;
    m_LeadZeroPos = m_NumDigits;
    for(i=m_NumDigits-1; i>=m_DigStart; i--)
    {
        val = (int)(rem/m_DigitInfo[i].weight);
        if(m_DigitInfo[i].val != val)
        {
            m_DigitInfo[i].val = val;
            m_DigitInfo[i].modified = TRUE;
        }
        rem = rem - val*m_DigitInfo[i].weight;
        acc += val;
        if( (acc==0) && ( i>m_DecPos) )
            m_LeadZeroPos = i;
    }
    // signal the new frequency to world
    m_Oldfreq = m_freq;
    if(emitSignal)
        emit NewFrequency( m_freq );
    UpdateCtrl(m_LastLeadZeroPos != m_LeadZeroPos);
    m_LastLeadZeroPos = m_LeadZeroPos;
}

//////////////////////////////////////////////////////////////////////////////
//  Sets the Digit and comma and decimal pt color
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetDigitColor(QColor cr)
{
    m_UpdateAll = TRUE;
    m_DigitColor = cr;
    for(int i=m_DigStart; i<m_NumDigits; i++)
        m_DigitInfo[i].modified = TRUE;
    UpdateCtrl(TRUE);
}

//////////////////////////////////////////////////////////////////////////////
//  Sets the Digit units
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetUnits(FUNITS units)
{
    switch(units)
    {
        case UNITS_HZ:
            m_DecPos = 0;
            m_UnitString = "Hz ";
            break;
        case UNITS_KHZ:
            m_DecPos = 3;
            m_UnitString = "KHz";
            break;
        case UNITS_MHZ:
            m_DecPos = 6;
            m_UnitString = "MHz";
            break;
        case UNITS_GHZ:
            m_DecPos = 9;
            m_UnitString = "GHz";
            break;
        case UNITS_SEC:
            m_DecPos = 6;
            m_UnitString = "Sec";
            break;
        case UNITS_MSEC:
            m_DecPos = 3;
            m_UnitString = "mS ";
            break;
        case UNITS_USEC:
            m_DecPos = 0;
            m_UnitString = "uS ";
            break;
        case UNITS_NSEC:
            m_DecPos = 0;
            m_UnitString = "nS ";
            break;
    }
    m_UpdateAll = TRUE;
    UpdateCtrl(TRUE);
}
//////////////////////////////////////////////////////////////////////////////
//  Sets the Background color
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetBkColor(QColor cr)
{
    m_UpdateAll = TRUE;
    m_BkColor = cr;
    for(int i=m_DigStart; i<m_NumDigits; i++)
        m_DigitInfo[i].modified = TRUE;
    UpdateCtrl(TRUE);
}

//////////////////////////////////////////////////////////////////////////////
//  Sets the Units text color
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetUnitsColor(QColor cr)
{
    m_UpdateAll = TRUE;
    m_UnitsColor = cr;
    UpdateCtrl(TRUE);
}

//////////////////////////////////////////////////////////////////////////////
//  Sets the Mouse edit selection text background color
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::SetHighlightColor(QColor cr)
{
    m_UpdateAll = TRUE;
    m_HighlightColor = cr;
    UpdateCtrl(TRUE);
}


//////////////////////////////////////////////////////////////////////////////
//  call to force control to redraw
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::UpdateCtrl(bool all)
{
    if(all)
    {
        m_UpdateAll = TRUE;
        for(int i=m_DigStart; i<m_NumDigits; i++)
            m_DigitInfo[i].modified = TRUE;
    }
    update();
}


/////////////////////////////////////////////////////////////////////
//  Various Event overrides
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::resizeEvent(QResizeEvent* )
{
//qDebug() <<rect.width() << rect.height();
    m_Pixmap = QPixmap(size());     //resize pixmap to current control size
    m_Pixmap.fill(m_BkColor);
    m_UpdateAll = TRUE;
}

void CFreqCtrl::leaveEvent( QEvent *  )
{   //called when mouse cursor leaves this control so deactivate any highlights
    if(m_ActiveEditDigit>=0)
    {
        if( m_DigitInfo[m_ActiveEditDigit].editmode )
        {
            m_DigitInfo[m_ActiveEditDigit].editmode = FALSE;
            m_DigitInfo[m_ActiveEditDigit].modified = TRUE;
            m_ActiveEditDigit = -1;
            UpdateCtrl(FALSE);
        }
    }
}

/////////////////////////////////////////////////////////////////////
//  main draw event for this control
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::paintEvent(QPaintEvent *)
{
    QPainter painter(&m_Pixmap);
    if(m_UpdateAll) //if need to redraw everything
    {
        DrawBkGround(painter);
        m_UpdateAll = FALSE;
    }
    // draw any modified digits to the m_MemDC
    DrawDigits(painter);
    //now draw pixmap onto screen
    QPainter scrnpainter(this);
    scrnpainter.drawPixmap(0,0,m_Pixmap);   //blt to the screen(flickers like a candle, why?)
}

/////////////////////////////////////////////////////////////////////
//  Mouse Event overrides
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::mouseMoveEvent(QMouseEvent * event)
{
    return;
    QPoint pt = event->pos();
    //find which digit is to be edited
    if( isActiveWindow() )
    {
        if(!hasFocus())
            setFocus(Qt::MouseFocusReason);
        for(int i=m_DigStart; i<m_NumDigits; i++)
        {
            if( InRect( m_DigitInfo[i].dQRect, pt) )
            {
                if( !m_DigitInfo[i].editmode )
                {
                    m_DigitInfo[i].editmode = TRUE;
                    m_ActiveEditDigit = i;
                }
            }
            else
            {       //un-highlight the previous digit if moved off it
                if( m_DigitInfo[i].editmode )
                {
                    m_DigitInfo[i].editmode = FALSE;
                    m_DigitInfo[i].modified = TRUE;
                }
            }
        }
        UpdateCtrl(FALSE);
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Service mouse button clicks to inc or dec the selected frequency
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::mousePressEvent(QMouseEvent * event)
{
    return;
QPoint pt = event->pos();
    if(event->button() == Qt::LeftButton)
    {
        for(int i=m_DigStart; i<m_NumDigits; i++)
        {
            if( InRect( m_DigitInfo[i].dQRect, pt) )    //if in i'th digit
            {
                if( m_LRMouseFreqSel )
                {
                    DecFreq();
                }
                else
                {
                    if(pt.y() < m_DigitInfo[i].dQRect.bottom()/2)   //top half?
                        IncFreq();
                    else
                        DecFreq();          //bottom half
                }
            }
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        for(int i=m_DigStart; i<m_NumDigits; i++)
        {
            if( InRect( m_DigitInfo[i].dQRect, pt) )    //if in i'th digit
            {
                if( m_LRMouseFreqSel )
                {
                    IncFreq();
                }
                else
                {
                    if(pt.y() < m_DigitInfo[i].dQRect.bottom()/2)   //top half?
                        IncDigit();
                    else
                        DecDigit();         //botom half
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
//  Mouse Wheel Event overrides
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::wheelEvent(QWheelEvent *event)
{
    QPointF pt = event->position();
    int numDegrees = event->pixelDelta().manhattanLength() / 8;
    int numSteps = numDegrees / 15;

    for (int i = m_DigStart; i < m_NumDigits; i++)
    {
        if (InRect(m_DigitInfo[i].dQRect, pt.toPoint())) // pass pt.toPoint() directly
        {
            if (numSteps > 0)
                IncFreq();
            else if (numSteps < 0)
                DecFreq();
        }
    }
}

/////////////////////////////////////////////////////////////////////
//  Keyboard Event overrides
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::keyPressEvent( QKeyEvent * event )
{   //call base class if dont over ride key
bool fSkipMsg = FALSE;
qint64 tmp;
//qDebug() <<event->key();
    switch(event->key())
    {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            if( m_ActiveEditDigit>=0)
            {
                if( m_DigitInfo[m_ActiveEditDigit].editmode)
                {
                    tmp = (m_freq/m_DigitInfo[m_ActiveEditDigit].weight)%10;
                    m_freq -= tmp*m_DigitInfo[m_ActiveEditDigit].weight;
                    m_freq = m_freq+(event->key()-'0')*m_DigitInfo[m_ActiveEditDigit].weight;
                    SetFrequency(m_freq);
                }
            }
            MoveCursorRight();
            fSkipMsg = TRUE;
            break;
        case Qt::Key_Left:
            if( m_ActiveEditDigit != -1 )
            {
                MoveCursorLeft();
                fSkipMsg = TRUE;
            }
            break;
        case Qt::Key_Up:
            if(m_ActiveEditDigit != -1 )
            {
                IncFreq();
                fSkipMsg = TRUE;
            }
            break;
        case Qt::Key_Down:
            if(m_ActiveEditDigit != -1)
            {
                DecFreq();
                fSkipMsg = TRUE;
            }
            break;
        case Qt::Key_Right:
            if(m_ActiveEditDigit != -1 )
            {
                MoveCursorRight();
                fSkipMsg = TRUE;
            }
            break;
        case Qt::Key_Home:
            CursorHome();
            fSkipMsg = TRUE;
            break;
        case Qt::Key_End:
            CursorEnd();
            fSkipMsg = TRUE;
            break;
        default:
            break;
    }
    if(!fSkipMsg)
        QFrame::keyPressEvent(event);
}


//////////////////////////////////////////////////////////////////////////////
//  Calculates all the rectangles for the digits, separators, and units text
//    and creates the fonts for them.
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::DrawBkGround(QPainter &Painter)
{
QRect rect(0, 0, width(), height());
//qDebug() <<rect;

int cellwidth = 100*rect.width()/(100*(m_NumDigits+g_constant)+(m_NumSeps*SEPRATIO_N)/SEPRATIO_D);
int sepwidth = (SEPRATIO_N*cellwidth)/(100*SEPRATIO_D);
//qDebug() <<cellwidth <<sepwidth;

    m_UnitsRect.setRect(rect.right() - g_constant*cellwidth,
                        rect.top(),
                        g_constant*cellwidth,
                        rect.height());
    Painter.fillRect(m_UnitsRect, m_BkColor);
//draw units text
    m_UnitsFont.setPixelSize( (UNITS_SIZE_PERCENT*rect.height())/100 );
    m_UnitsFont.setFamily("Arial");
    Painter.setFont(m_UnitsFont );
    Painter.setPen(m_UnitsColor);
    Painter.drawText(m_UnitsRect, Qt::AlignHCenter|Qt::AlignVCenter, m_UnitString);


    m_DigitFont.setPixelSize( (DIGIT_SIZE_PERCENT*rect.height())/100 );
    m_DigitFont.setFamily("Arial");
    Painter.setFont(m_DigitFont );
    Painter.setPen(m_DigitColor);

    int digpos = rect.right() - g_constant*cellwidth;    //starting digit x position
    for(int i=m_DigStart; i<m_NumDigits; i++)
    {
        if( (i>m_DigStart) && ( (i%3)==0 ) )
        {
            m_SepRect[i].setCoords( digpos - sepwidth,
                                    rect.top(),
                                    digpos,
                                    rect.bottom());
            Painter.fillRect(m_SepRect[i], m_BkColor);
            digpos -= sepwidth;
            if( i==m_DecPos)
                Painter.drawText(m_SepRect[i], Qt::AlignHCenter|Qt::AlignVCenter, ".");
            else
                if( i>m_DecPos && i<m_LeadZeroPos)
                    Painter.drawText(m_SepRect[i], Qt::AlignHCenter|Qt::AlignVCenter, ".");
                else
                    if( i<m_LeadZeroPos)
                        Painter.drawText(m_SepRect[i], Qt::AlignHCenter|Qt::AlignVCenter, " ");
        }
        else
        {
            m_SepRect[i].setCoords( 0,0,0,0 );
        }
        m_DigitInfo[i].dQRect.setCoords(digpos - cellwidth,
                                        rect.top(),
                                        digpos,
                                        rect.bottom());
        digpos -= cellwidth;
    }

}

//////////////////////////////////////////////////////////////////////////////
//  Draws just the Digits that have been modified
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::DrawDigits(QPainter &Painter)
{
char digchar;
    Painter.setFont(m_DigitFont );
    m_FirstEditableDigit = m_DigStart;
//qDebug() <<m_DigStart <<m_LeadZeroPos;
    for(int i=m_DigStart; i<m_NumDigits; i++)
    {
        if(m_DigitInfo[i].incval == 0)
            m_FirstEditableDigit++;
        if( m_DigitInfo[i].modified || m_DigitInfo[i].editmode )
        {
            if( m_DigitInfo[i].editmode && m_DigitInfo[i].incval != 0 )
                Painter.fillRect(m_DigitInfo[i].dQRect, m_HighlightColor);
            else
                Painter.fillRect(m_DigitInfo[i].dQRect, m_BkColor);
            if(i >= m_LeadZeroPos)
                Painter.setPen(m_BkColor);
            else
                Painter.setPen(m_DigitColor);
            digchar = '0' + m_DigitInfo[i].val;
            Painter.drawText(m_DigitInfo[i].dQRect, Qt::AlignHCenter|Qt::AlignVCenter, QString().number( m_DigitInfo[i].val ) );
            m_DigitInfo[i].modified = FALSE;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Increment just the digit active in edit mode
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::IncDigit()
{
int tmp;
qint64 tmpl;
    if( m_ActiveEditDigit>=0)
    {
        if( m_DigitInfo[m_ActiveEditDigit].editmode)
        {
            if(m_DigitInfo[m_ActiveEditDigit].weight == m_DigitInfo[m_ActiveEditDigit].incval)
            {
                // get the current digit value
                tmp = (int)((m_freq/m_DigitInfo[m_ActiveEditDigit].weight)%10);
                // set the current digit value to zero
                m_freq -= tmp*m_DigitInfo[m_ActiveEditDigit].weight;
                tmp++;
                if( tmp>9 )
                    tmp = 0;
                m_freq = m_freq+(qint64)tmp*m_DigitInfo[m_ActiveEditDigit].weight;
            }
            else
            {
                tmp = (int)((m_freq/m_DigitInfo[m_ActiveEditDigit+1].weight)%10);
                tmpl = m_freq + m_DigitInfo[m_ActiveEditDigit].incval;
                if(tmp != (int)((tmpl/m_DigitInfo[m_ActiveEditDigit+1].weight)%10) )
                {
                    tmpl -= m_DigitInfo[m_ActiveEditDigit+1].weight;
                }
                m_freq = tmpl;
            }
            SetFrequency(m_freq);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Increment the frequency by this digit active in edit mode
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::IncFreq()
{
    if( m_ActiveEditDigit>=0)
    {
        if( m_DigitInfo[m_ActiveEditDigit].editmode)
        {
            m_freq += m_DigitInfo[m_ActiveEditDigit].incval;
            if (m_ResetLowerDigits) {
                /* Set digits below the active one to 0 */
                m_freq = m_freq - m_freq%m_DigitInfo[m_ActiveEditDigit].weight;
            }
            SetFrequency(m_freq);
            m_LastEditDigit = m_ActiveEditDigit;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Decrement the digit active in edit mode
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::DecDigit()
{
int tmp;
qint64 tmpl;
    if( m_ActiveEditDigit>=0)
    {
        if( m_DigitInfo[m_ActiveEditDigit].editmode)
        {
            if(m_DigitInfo[m_ActiveEditDigit].weight == m_DigitInfo[m_ActiveEditDigit].incval)
            {
                // get the current digit value
                tmp = (int)((m_freq/m_DigitInfo[m_ActiveEditDigit].weight)%10);
                // set the current digit value to zero
                m_freq -= tmp*m_DigitInfo[m_ActiveEditDigit].weight;
                tmp--;
                if( tmp<0 )
                    tmp = 9;
                m_freq = m_freq+(qint64)tmp*m_DigitInfo[m_ActiveEditDigit].weight;
            }
            else
            {
                tmp = (int)((m_freq/m_DigitInfo[m_ActiveEditDigit+1].weight)%10);
                tmpl = m_freq - m_DigitInfo[m_ActiveEditDigit].incval;
                if(tmp != (int)((tmpl/m_DigitInfo[m_ActiveEditDigit+1].weight)%10) )
                {
                    tmpl += m_DigitInfo[m_ActiveEditDigit+1].weight;
                }
                m_freq = tmpl;
            }
            SetFrequency(m_freq);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////
//  Decrement the frequency by this digit active in edit mode
//////////////////////////////////////////////////////////////////////////////
void CFreqCtrl::DecFreq()
{
    if( m_ActiveEditDigit>=0)
    {
        if( m_DigitInfo[m_ActiveEditDigit].editmode)
        {
            m_freq -= m_DigitInfo[m_ActiveEditDigit].incval;
            if (m_ResetLowerDigits) {
                /* digits below the active one are reset to 0 */
                m_freq = m_freq - m_freq%m_DigitInfo[m_ActiveEditDigit].weight;
            }

            SetFrequency(m_freq);
            m_LastEditDigit = m_ActiveEditDigit;
        }
    }
}

/////////////////////////////////////////////////////////////////////
//  Cursor move routines for arrow key editing
/////////////////////////////////////////////////////////////////////
void CFreqCtrl::MoveCursorLeft()
{
QPoint pt;
    if( (m_ActiveEditDigit >=0) && (m_ActiveEditDigit<m_NumDigits-1) )
    {
        cursor().setPos(mapToGlobal( m_DigitInfo[++m_ActiveEditDigit].dQRect.center() ));
    }
}

void CFreqCtrl::MoveCursorRight()
{
QPoint pt;
    if( m_ActiveEditDigit > m_FirstEditableDigit )
    {
        cursor().setPos( mapToGlobal( m_DigitInfo[--m_ActiveEditDigit].dQRect.center() ) );
    }
}

void CFreqCtrl::CursorHome()
{
QPoint pt;
    if( m_ActiveEditDigit >= 0 )
    {
        cursor().setPos( mapToGlobal( m_DigitInfo[m_NumDigits-1].dQRect.center() ) );
    }
}

void CFreqCtrl::CursorEnd()
{
QPoint pt;
    if( m_ActiveEditDigit > 0 )
    {
        cursor().setPos( mapToGlobal( m_DigitInfo[m_FirstEditableDigit].dQRect.center() ) );
    }
}
