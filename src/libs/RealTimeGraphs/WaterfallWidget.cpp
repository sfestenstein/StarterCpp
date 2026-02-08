#include "RealTimeGraphs/WaterfallWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace RealTimeGraphs
{

// ============================================================================
// Construction
// ============================================================================

WaterfallWidget::WaterfallWidget(int historyRows, QWidget* parent)
   : QWidget(parent)
   , _historyRows{historyRows}
   , _rows(static_cast<std::size_t>(historyRows))
{
   setMinimumSize(minimumSizeHint());
   setAttribute(Qt::WA_OpaquePaintEvent);
}

// ============================================================================
// Public API
// ============================================================================

void WaterfallWidget::addRow(const std::vector<float>& magnitudes)
{
   // Normalise the incoming row
   std::vector<float> normRow;
   normRow.reserve(magnitudes.size());
   for (float val : magnitudes)
   {
      normRow.push_back(toNormalised(val));
   }

   {
      std::lock_guard<std::mutex> lock(_mutex);
      _binCount = static_cast<int>(magnitudes.size());
      _rows.push(normRow);
   }
   update();
}

void WaterfallWidget::setDbRange(float minDb, float maxDb)
{
   _minDb = minDb;
   _maxDb = maxDb;
   update();
}

void WaterfallWidget::setInputIsDb(bool isDb)
{
   _inputIsDb = isDb;
   update();
}

void WaterfallWidget::setColorMap(ColorMap::Palette palette)
{
   _colorMap = ColorMap(palette);
   update();
}

void WaterfallWidget::setFrequencyRange(double centerFreqHz, double bandwidthHz)
{
   _centerFreqHz = centerFreqHz;
   _bandwidthHz  = bandwidthHz;
   update();
}

QSize WaterfallWidget::minimumSizeHint() const
{
   return {320, 200};
}

// ============================================================================
// Events
// ============================================================================

void WaterfallWidget::paintEvent(QPaintEvent* /*event*/)
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, false);

   QRect plotArea(MARGIN_LEFT, MARGIN_TOP,
                  width() - MARGIN_LEFT - MARGIN_RIGHT,
                  height() - MARGIN_TOP - MARGIN_BOTTOM);

   // Background
   painter.fillRect(rect(), QColor(25, 25, 30));

   // Build and draw the spectrogram image
   rebuildImage();

   if (!_image.isNull())
   {
      painter.drawImage(plotArea, _image);
   }
   else
   {
      painter.fillRect(plotArea, QColor(15, 15, 20));
   }

   // Draw border around plot area
   painter.setPen(QColor(60, 60, 70));
   painter.drawRect(plotArea);

   // Labels
   painter.setPen(QColor(180, 180, 190));
   QFont font = painter.font();
   font.setPointSize(8);
   painter.setFont(font);

   // Y-axis: time labels
   painter.drawText(0, plotArea.top() - 6, MARGIN_LEFT - 5, 12,
                    Qt::AlignRight | Qt::AlignVCenter, "Now");
   painter.drawText(0, plotArea.bottom() - 6, MARGIN_LEFT - 5, 12,
                    Qt::AlignRight | Qt::AlignVCenter, "Oldest");

   // X-axis: frequency tick labels
   drawFrequencyLabels(painter, plotArea);
}

void WaterfallWidget::resizeEvent(QResizeEvent* event)
{
   QWidget::resizeEvent(event);
   update();
}

// ============================================================================
// Internals
// ============================================================================

void WaterfallWidget::rebuildImage()
{
   std::lock_guard<std::mutex> lock(_mutex);

   auto rowCount = static_cast<int>(_rows.size());
   if (rowCount == 0 || _binCount == 0)
   {
      _image = QImage();
      return;
   }

   // Create an image with one pixel per bin horizontally, one per row vertically.
   // Most recent row is at top (row index 0 in image), oldest at bottom.
   _image = QImage(_binCount, rowCount, QImage::Format_RGBA8888);

   for (int r = 0; r < rowCount; ++r)
   {
      // Logical index: 0 = oldest.  We want newest at image top.
      int logicalIdx = rowCount - 1 - r;
      const auto& row = _rows[static_cast<std::size_t>(logicalIdx)];

      auto* scanLine = reinterpret_cast<uint8_t*>(_image.scanLine(r));
      int cols = std::min(_binCount, static_cast<int>(row.size()));

      for (int c = 0; c < cols; ++c)
      {
         Color clr = _colorMap.map(row[static_cast<std::size_t>(c)]);
         int offset = c * 4;
         scanLine[offset + 0] = clr.r;
         scanLine[offset + 1] = clr.g;
         scanLine[offset + 2] = clr.b;
         scanLine[offset + 3] = clr.a;
      }
   }
}

float WaterfallWidget::toNormalised(float value) const
{
   float db = value;
   if (!_inputIsDb)
   {
      constexpr float EPSILON = 1.0e-10F;
      db = 20.0F * std::log10(std::max(value, EPSILON));
   }

   float norm = (db - _minDb) / (_maxDb - _minDb);
   return std::clamp(norm, 0.0F, 1.0F);
}

void WaterfallWidget::drawFrequencyLabels(QPainter& painter, const QRect& area)
{
   painter.setPen(QColor(180, 180, 190));
   QFont font = painter.font();
   font.setPointSize(8);
   painter.setFont(font);

   constexpr int X_TICKS = 8;
   bool hasFreq = (_bandwidthHz > 0.0);
   double startFreq = _centerFreqHz - _bandwidthHz / 2.0;

   for (int i = 0; i <= X_TICKS; ++i)
   {
      float frac = static_cast<float>(i) / static_cast<float>(X_TICKS);
      int xPos = area.left() + static_cast<int>(frac * static_cast<float>(area.width()));

      QString label;
      if (hasFreq)
      {
         double freq = startFreq + static_cast<double>(frac) * _bandwidthHz;
         label = QString::fromStdString(formatFrequency(freq));
      }
      else
      {
         label = QString::number(i);
      }

      constexpr int LABEL_WIDTH = 80;
      painter.drawText(xPos - LABEL_WIDTH / 2, area.bottom() + 3,
                       LABEL_WIDTH, MARGIN_BOTTOM - 5,
                       Qt::AlignCenter, label);
   }
}

std::string WaterfallWidget::formatFrequency(double freqHz)
{
   double absFreq = std::abs(freqHz);
   char buf[32];

   if (absFreq >= 1.0e9)
   {
      std::snprintf(buf, sizeof(buf), "%.3f GHz", freqHz / 1.0e9);
   }
   else if (absFreq >= 1.0e6)
   {
      std::snprintf(buf, sizeof(buf), "%.3f MHz", freqHz / 1.0e6);
   }
   else if (absFreq >= 1.0e3)
   {
      std::snprintf(buf, sizeof(buf), "%.3f kHz", freqHz / 1.0e3);
   }
   else
   {
      std::snprintf(buf, sizeof(buf), "%.1f Hz", freqHz);
   }
   return std::string(buf);
}

} // namespace RealTimeGraphs
