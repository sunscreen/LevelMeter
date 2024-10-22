#include "levelmeter.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>
#include <cmath>

// Constants
const int RedrawInterval = 100; // ms
const qreal PeakDecayRate = 0.001;
const int PeakHoldLevelDuration = 2000; // ms


LevelMeter::LevelMeter(QWidget *parent)
    : QWidget(parent),
      m_rmsLevel(0.0),
      m_peakLevel(0.0),
      m_decayedPeakLevel(0.0),
      m_peakDecayRate(PeakDecayRate),
      m_peakHoldLevel(0.0),
      m_redrawTimer(new QTimer(this)),
      m_rmsColor(Qt::red),
      m_peakColor(255, 200, 200, 255)
{
    setMinimumWidth(15);

    connect(m_redrawTimer, &QTimer::timeout, this, &LevelMeter::redrawTimerExpired);

    decay_path = QPainterPath();
    rms_path = QPainterPath();
    clear_segs_path = QPainterPath();

    white.setRgb(255,255,255,128);
    red.setRgb(200,0,0);
    green.setRgb(0,200,0);
    gray.setRgb(99,99,99);

    solidpen.setStyle(Qt::SolidLine);
    solidpen.setColor(QColor(99,99,99,128));
    solidpen.setWidthF(0.5);

    segment_pen.setStyle(Qt::SolidLine);
    segment_pen.setColor(QColor(0,0,0,128));
    segment_pen.setWidthF(0.5);

    clearbrush.setStyle(Qt::SolidPattern);
    clearbrush.setColor(gray);

    peakhold_brush.setStyle(Qt::SolidPattern);
    peakhold_brush.setColor(green);
    rms_brush.setStyle(Qt::SolidPattern);

    peakdecay_brush.setStyle(Qt::SolidPattern);

}

LevelMeter::~LevelMeter() = default;

void LevelMeter::reset()
{
    m_rmsLevel = 0.0;
    m_peakLevel = 0.0;
}

void LevelMeter::levelChanged(qreal rmsLevel, float peakLevel, int numSamples)
{
    // Smooth the RMS signal
    //const qreal smooth = pow(qreal(0.9), static_cast<qreal>(numSamples) / 256); // TODO: remove this magic number
    //m_rmsLevel = (m_rmsLevel * smooth) + (rmsLevel * (1.0 - smooth));
    m_rmsLevel=rmsLevel;

    if ( m_redrawTimer->isActive() == false ) {
    m_redrawTimer->start(RedrawInterval);
    }

    if (peakLevel > m_decayedPeakLevel) {
        m_peakLevel = peakLevel;
        m_decayedPeakLevel = peakLevel;
        qreal decay_segments = floor((bar_dist / 6) * m_decayedPeakLevel);
        m_decayedPeak_seg=decay_segments;

        m_peakLevelChanged.start();
    }

    if (peakLevel > m_peakHoldLevel) {
        qreal decay_segments = floor((bar_dist / 6) * m_decayedPeakLevel);

        m_peakHold_seg = decay_segments+6;
        m_peakHoldLevel = peakLevel;
        m_peakHoldLevelChanged.start();
    }

    update();
}

void LevelMeter::redrawTimerExpired()
{

   if (m_peakLevelChanged.isValid() == false ) { qDebug() << "timer not valid"; return; }
    const int elapsedMs = m_peakLevelChanged.elapsed();
    const qreal decayAmount = m_peakDecayRate * elapsedMs;

    if (decayAmount < m_peakLevel) {
        m_decayedPeakLevel = m_peakLevel - decayAmount;
        //if (m_peakHold_seg < m_decayedPeak_seg) m_peakHold_seg -= decayAmount * 6;
    } else {
        m_decayedPeakLevel = 0.0;
    }

    // Check whether to clear the peak hold level
    if (m_peakHoldLevelChanged.elapsed() > PeakHoldLevelDuration) {
        m_peakHoldLevel = 0.0;
        m_peakHold_seg=0;
    }
    //update();
}


void LevelMeter::draw_decaypeak(QPainter &painter,QRect &bar) {

    qreal decay_segments = floor((bar_dist / 6) * m_decayedPeakLevel);
    qreal startp = rect().bottom();

    int glum=85;
    int ghue=100;
    int gsat=255;

    int yhue=70;
    int ysat=255;
    int ylum=125;

    int rhue=5;
    int rsat=255;
    int rlum=125;

    for (int x=0;x<decay_segments;x++) {
   
    bar.setBottom(startp);
    bar.setTop(startp-5);
    green_hsl.setHsl(ghue,gsat,glum);
    //ghue+=1;
    //gsat-=1;
    //glum+=1;

    //for (int l=0;l<x/3;l++) green_hsl=green_hsl.lighter();
    //green_hsl=green_hsl.lighter();

    decay_path.clear();
    decay_path.addRoundedRect(QRectF(bar), 2, 2);

    peakdecay_brush.setColor(white);
    painter.setPen(Qt::NoPen);
    painter.setBrush(peakdecay_brush);

    painter.setClipPath(decay_path);
    painter.drawRect(bar);

    painter.setPen(solidpen);
    peakdecay_brush.setColor(green_hsl);
    painter.setBrush(peakdecay_brush);

/*    if ((startp) < (rect().height()/2)) {

        //QColor yellow_power(255,ypwr,0);
        QColor yellow_hsl;
        yellow_hsl.setHsl(yhue,ysat,ypwr);
        bdashrec_decay.setColor(yellow_hsl);
        painter.setBrush(bdashrec_decay);

     }
*/

    if ((startp) < (rect().height()/5)) {

        QColor red_hsl;
        red_hsl.setHsl(rhue, rsat,rlum);
        //red_hsl=red_hsl.lighter();
        //rhue-=1;
        //rlum-=1;
        //rsat-=1;
        peakdecay_brush.setColor(red_hsl);
        painter.setBrush(peakdecay_brush);

     }

    painter.drawRect(bar);
    startp-=6;
   }



}


void LevelMeter::draw_rmspower(QPainter &painter,QRect &bar) {


    qreal rms_segments = floor((bar_dist / 6 * m_rmsLevel));
    //qDebug() << rms_segments;
    if (rms_segments < 1) return;


    qreal startp=rect().bottom();
    int ohue=23;
    int osat=225;
    int olum=125;

    for (int x=0;x<rms_segments;x++) {
    orange_hsl.setHsl(ohue,osat,olum);

    //olum-=1;
    //osat-=1;

    bar.setBottom(startp);
    bar.setTop(startp-5);

    rms_path.clear();
    rms_path.addRoundedRect(QRectF(bar), 2, 2);

    rms_brush.setColor(white);
    painter.setPen(Qt::NoPen);
    painter.setBrush(rms_brush);

    painter.setClipPath(rms_path);
    painter.drawRect(bar);

    rms_brush.setColor(orange_hsl);
    painter.setPen(solidpen);
    painter.setBrush(rms_brush);
    painter.drawRect(bar);
    startp-=6;
    }

}
void LevelMeter::draw_peakhold(QPainter &painter,QRect &bar) {


    //qreal hold_pos = floor(rect().top()*7 + (1.0 - m_peakHoldLevel) * rect().height())-6;
    qreal hold_pos = rect().bottom() - (m_peakHold_seg * 5);
    //qDebug() << hold_pos;

    painter.setPen(Qt::NoPen);
    painter.setBrush(peakhold_brush);

    if (hold_pos < bar.height() / 6) {
    peakhold_brush.setColor(red);
    painter.setBrush(peakhold_brush);
    }

    bar.setTop(hold_pos);
    bar.setBottom(hold_pos + 5);
    painter.drawRect(bar);

}

void LevelMeter::clearMeter(QPainter &painter,QRect clearbar) {
    painter.setBrush(clearbrush);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    qreal startp=clearbar.bottom();
    int max_segments = floor(clearbar.height()/6);

    painter.setPen(segment_pen);

    for (int x=0;x<max_segments;x++) {
    clearbar.setBottom(startp);
    clearbar.setTop(startp-5);

    clear_segs_path.clear();
    clear_segs_path.addRoundedRect(QRectF(clearbar), 2, 2);
    painter.setClipPath(clear_segs_path);
    painter.drawRect(clearbar);
    startp-=6;
    }

}


void LevelMeter::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect bar = rect();
    bar_dist = rect().height();

    painter.save();
    clearMeter(painter,bar);
    painter.restore();

    painter.save();
    draw_decaypeak(painter,bar);
    draw_rmspower(painter,bar);
    painter.restore();

    painter.save();
    draw_peakhold(painter,bar);
    painter.restore();

    return;

}
