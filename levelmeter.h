#ifndef LEVELMETER_H
#define LEVELMETER_H

#include <QElapsedTimer>
#include <QWidget>
#include <QPainterPath>
#include <QPen>

/**
 * Widget which displays a vertical audio level meter, indicating the
 * RMS and peak levels of the window of audio samples most recently analyzed
 * by the Engine.
 */
class LevelMeter : public QWidget
{
    Q_OBJECT

public:
    explicit LevelMeter(QWidget *parent = nullptr);
    ~LevelMeter();

    void paintEvent(QPaintEvent *event) override;
    bool m_resetstate=false;
    void clearMeter(QPainter &painter,QRect clearbar);
    void draw_peakhold(QPainter &painter,QRect &bar);
    void draw_rmspower(QPainter &painter,QRect &bar);
    void draw_decaypeak(QPainter &painter,QRect &bar);

public slots:
    void reset();
    void levelChanged(qreal rmsLevel, float peakLevel, int numSamples);

private slots:
    void redrawTimerExpired();

private:
    qreal bar_dist;
    /**
     * Height of RMS level bar.
     * Range 0.0 - 1.0.
     */
    qreal m_rmsLevel;

    /**
     * Most recent peak level.
     * Range 0.0 - 1.0.
     */
    qreal m_peakLevel;

    /**
     * Height of peak level bar.
     * This is calculated by decaying m_peakLevel depending on the
     * elapsed time since m_peakLevelChanged, and the value of m_decayRate.
     */
    qreal m_decayedPeakLevel;
    qreal m_decayedPeak_seg;
    /**
     * Time at which m_peakLevel was last changed.
     */
    QElapsedTimer m_peakLevelChanged;

    /**
     * Rate at which peak level bar decays.
     * Expressed in level units / millisecond.
     */
    qreal m_peakDecayRate;

    /**
     * High watermark of peak level.
     * Range 0.0 - 1.0.
     */
    qreal m_peakHoldLevel;
    qreal m_peakHold_seg;

    /**
     * Time at which m_peakHoldLevel was last changed.
     */
    QElapsedTimer m_peakHoldLevelChanged;

    QTimer *m_redrawTimer;

    QColor m_rmsColor;
    QColor m_peakColor;

    QColor orange_hsl;
    QColor yellow_hsl;
    QColor green_hsl;
    QColor red_hsl;

    QColor red;
    QColor green;
    QColor white;
    QColor gray;

    QPainterPath rms_path;
    QPainterPath decay_path;
    QPainterPath clear_segs_path;
    QPen solidpen;
    QPen segment_pen;

    QBrush clearbrush;
    QBrush peakhold_brush;





//    QPainterPath decay_path;

};

#endif // LEVELMETER_H
