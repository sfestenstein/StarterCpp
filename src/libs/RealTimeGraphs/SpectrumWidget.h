#ifndef SPECTRUMWIDGET_H_
#define SPECTRUMWIDGET_H_

// Project headers
#include "RealTimeGraphs/ColorMap.h"

// Third-party headers
#include <QWidget>

// System headers
#include <mutex>
#include <vector>

namespace RealTimeGraphs
{

/// Custom QPainter-based spectrum / FFT bar-chart widget.
///
/// Renders a horizontal frequency axis and a vertical amplitude axis.
/// Each bin is drawn as a filled vertical bar whose colour comes from
/// the active ColorMap.
///
/// Call `setData()` from any thread; the widget double-buffers the data
/// and schedules a repaint.
class SpectrumWidget : public QWidget
{
   Q_OBJECT

public:
   explicit SpectrumWidget(QWidget* parent = nullptr);

   /// Replace the current spectrum data.  The vector is copied.
   /// Thread-safe — can be called from a producer thread.
   /// @param magnitudes  Linear magnitudes (0 … 1 normalised).
   void setData(const std::vector<float>& magnitudes);

   /// Set the dB display range (e.g., -120 to 0).
   void setDbRange(float minDb, float maxDb);

   /// Set whether data is already in dB (true) or linear (false).
   void setInputIsDb(bool isDb);

   /// Change the colour palette.
   void setColorMap(ColorMap::Palette palette);

   /// Set number of horizontal grid lines.
   void setGridLines(int count);

   /// Minimum size hint for layout.
   [[nodiscard]] QSize minimumSizeHint() const override;

protected:
   void paintEvent(QPaintEvent* event) override;

private:
   void drawBackground(QPainter& painter, const QRect& area);
   void drawGrid(QPainter& painter, const QRect& area);
   void drawBars(QPainter& painter, const QRect& area);
   void drawLabels(QPainter& painter, const QRect& area);

   /// Convert a linear magnitude to normalised [0, 1] within the dB range.
   [[nodiscard]] float toNormalised(float value) const;

   std::mutex _mutex;
   std::vector<float> _data;

   ColorMap _colorMap{ColorMap::Palette::Viridis};
   float _minDb{-120.0F};
   float _maxDb{0.0F};
   bool _inputIsDb{false};
   int _gridLines{6};

   // Layout margins
   static constexpr int MARGIN_LEFT   = 50;
   static constexpr int MARGIN_RIGHT  = 10;
   static constexpr int MARGIN_TOP    = 10;
   static constexpr int MARGIN_BOTTOM = 30;
};

} // namespace RealTimeGraphs

#endif // SPECTRUMWIDGET_H_
