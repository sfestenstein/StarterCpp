#ifndef CONSTELLATIONWIDGET_H_
#define CONSTELLATIONWIDGET_H_

// Project headers
#include "CircularBuffer.h"

// Third-party headers
#include <QWidget>

// System headers
#include <complex>
#include <mutex>
#include <vector>

namespace RealTimeGraphs
{

/// Custom QPainter-based I/Q constellation diagram widget.
///
/// Renders a 2-D scatter plot of complex samples: the real part (I) on the
/// X-axis and the imaginary part (Q) on the Y-axis.  Recent points are
/// drawn brighter; older points fade toward the background.
class ConstellationWidget : public QWidget
{
   Q_OBJECT

public:
   /// @param historySize  Number of recent I/Q samples to display.
   /// @param parent       Parent widget.
   explicit ConstellationWidget(int historySize = 4096, QWidget* parent = nullptr);

   /// Replace the current constellation data.  Samples are appended to the
   /// persistence buffer.  Thread-safe.
   /// @param samples  Complex I/Q samples.
   void setData(const std::vector<std::complex<float>>& samples);

   /// Set the axis range (symmetric about the origin).
   void setAxisRange(float range);

   /// Set the point size in pixels.
   void setPointSize(int size);

   /// Enable or disable persistence (fading trail effect).
   void setPersistence(bool enable);

   /// Set persistence depth (how many samples to keep).
   void setPersistenceDepth(int depth);

   /// Set dot colour (recent samples).
   void setDotColor(const QColor& color);

   /// Enable or disable grid drawing.
   void setGridEnabled(bool enable);

   /// Minimum size hint for layout.
   [[nodiscard]] QSize minimumSizeHint() const override;

protected:
   void paintEvent(QPaintEvent* event) override;

private:
   void drawBackground(QPainter& painter, const QRect& area);
   void drawGrid(QPainter& painter, const QRect& area);
   void drawPoints(QPainter& painter, const QRect& area);

   /// Map an I/Q value to a pixel position within the plot area.
   [[nodiscard]] QPoint mapToPixel(float i, float q, const QRect& area) const;

   std::mutex _mutex;
   CommonUtils::CircularBuffer<std::complex<float>> _points;

   float _axisRange{1.5F};
   int _pointSize{3};
   bool _persistence{true};
   QColor _dotColor{0, 255, 128};
   bool _gridEnabled{true};

   // Layout margins
   static constexpr int MARGIN_LEFT   = 40;
   static constexpr int MARGIN_RIGHT  = 10;
   static constexpr int MARGIN_TOP    = 10;
   static constexpr int MARGIN_BOTTOM = 30;
};

} // namespace RealTimeGraphs

#endif // CONSTELLATIONWIDGET_H_
