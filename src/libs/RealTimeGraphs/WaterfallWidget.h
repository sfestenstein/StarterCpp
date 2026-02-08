#ifndef WATERFALLWIDGET_H_
#define WATERFALLWIDGET_H_

// Project headers
#include "CircularBuffer.h"
#include "RealTimeGraphs/ColorMap.h"

// Third-party headers
#include <QImage>
#include <QWidget>

// System headers
#include <mutex>
#include <string>
#include <vector>

namespace RealTimeGraphs
{

class ColorBarStrip; // forward declaration

/// Custom QPainter-based waterfall / spectrogram widget.
///
/// Each call to `addRow()` pushes a new frequency-domain row onto the
/// display.  The most recent row appears at the bottom; older rows scroll
/// upward and are eventually discarded.  Internally uses a `CircularBuffer`
/// of `QImage` scan lines.
class WaterfallWidget : public QWidget
{
   Q_OBJECT

public:
   /// @param historyRows  Number of time rows to keep in history.
   /// @param parent       Parent widget.
   explicit WaterfallWidget(int historyRows = 256, QWidget* parent = nullptr);

   /// Append a new spectrum row.  The vector is copied.
   /// Thread-safe — can be called from a producer thread.
   /// @param magnitudes  Linear magnitudes (0 … 1 normalised) or dB values.
   void addRow(const std::vector<float>& magnitudes);

   /// Set the dB display range (e.g., -120 to 0).
   void setDbRange(float minDb, float maxDb);

   /// Set whether data is already in dB (true) or linear (false).
   void setInputIsDb(bool isDb);

   /// Change the colour palette.
   void setColorMap(ColorMap::Palette palette);

   /// Set the frequency range so the x-axis shows real frequencies.
   /// @param centerFreqHz  Centre frequency in Hz.
   /// @param bandwidthHz   Total bandwidth in Hz.
   void setFrequencyRange(double centerFreqHz, double bandwidthHz);

   /// Show or hide the built-in colour-bar legend.
   void setColorBarVisible(bool visible);

   /// Minimum size hint for layout.
   [[nodiscard]] QSize minimumSizeHint() const override;

protected:
   void paintEvent(QPaintEvent* event) override;
   void resizeEvent(QResizeEvent* event) override;

private:
   /// Rebuild the off-screen image from circular buffer contents.
   void rebuildImage();

   /// Draw frequency tick labels along the x-axis.
   void drawFrequencyLabels(QPainter& painter, const QRect& area);

   /// Convert a frequency in Hz to a human-readable string (Hz/kHz/MHz/GHz).
   [[nodiscard]] static std::string formatFrequency(double freqHz);

   /// Convert a linear magnitude to normalised [0, 1] within the dB range.
   [[nodiscard]] float toNormalised(float value) const;

   std::mutex _mutex;

   int _historyRows;
   int _binCount{0};

   /// Each element is a spectrum row (vector of normalised values).
   CommonUtils::CircularBuffer<std::vector<float>> _rows;

   /// Off-screen rendered spectrogram image.
   QImage _image;

   ColorMap _colorMap{ColorMap::Palette::Viridis};
   float _minDb{-120.0F};
   float _maxDb{0.0F};
   bool _inputIsDb{false};

   double _centerFreqHz{0.0};
   double _bandwidthHz{0.0};

   // Embedded colour bar
   ColorBarStrip* _colorBar{nullptr};
   static constexpr int COLOR_BAR_WIDTH = 68;

   // Layout margins
   static constexpr int MARGIN_LEFT   = 55;
   static constexpr int MARGIN_RIGHT  = 83;  // COLOR_BAR_WIDTH + 15
   static constexpr int MARGIN_TOP    = 10;
   static constexpr int MARGIN_BOTTOM = 40;
};

} // namespace RealTimeGraphs

#endif // WATERFALLWIDGET_H_
