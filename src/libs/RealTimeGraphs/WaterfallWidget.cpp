#include "WaterfallWidget.h"

#include "ColorBarWidget.h"
#include "CommonGuiUtils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>
#include <cmath>

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
   setAttribute(Qt::WA_OpaquePaintEvent);
   setMinimumSize(320, 200);

   _colorBar = new ColorBarStrip(this);
   _colorBar->setDbRange(_minDb, _maxDb);
   _colorBar->setColorMap(_colorMap);
}

// ============================================================================
// Public API
// ============================================================================

void WaterfallWidget::addRow(const std::vector<float>& magnitudes)
{
   // Normalise the incoming row
   std::vector<float> normRow;
   normRow.reserve(magnitudes.size());
   for (const float val : magnitudes)
   {
      normRow.push_back(toNormalised(val));
   }

   {
      const std::lock_guard<std::mutex> lock(_mutex);
      _binCount = static_cast<int>(magnitudes.size());
      _rows.push(normRow);
   }
   update();
}

void WaterfallWidget::setDbRange(float minDb, float maxDb)
{
   _minDb = minDb;
   _maxDb = maxDb;
   _colorBar->setDbRange(minDb, maxDb);
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
   _colorBar->setColorMap(_colorMap);
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

void WaterfallWidget::setColorBarVisible(bool visible)
{
   _colorBar->setVisible(visible);
   update();
}

// ============================================================================
// Events
// ============================================================================

void WaterfallWidget::paintEvent(QPaintEvent* /*event*/)
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, false);

   const QRect plotArea(MARGIN_LEFT, MARGIN_TOP,
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

   const QRect area(MARGIN_LEFT, MARGIN_TOP,
              width() - MARGIN_LEFT - MARGIN_RIGHT,
              height() - MARGIN_TOP - MARGIN_BOTTOM);
   _colorBar->setGeometry(
      area.right() + 5,
      area.top(),
      COLOR_BAR_WIDTH,
      area.height());

   update();
}

// ============================================================================
// Internals
// ============================================================================

void WaterfallWidget::rebuildImage()
{
   const std::lock_guard<std::mutex> lock(_mutex);

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
      const int logicalIdx = rowCount - 1 - r;
      const auto& row = _rows[static_cast<std::size_t>(logicalIdx)];

      auto* scanLine = reinterpret_cast<uint8_t*>(_image.scanLine(r));
      const int cols = std::min(_binCount, static_cast<int>(row.size()));

      for (int c = 0; c < cols; ++c)
      {
         const Color clr = _colorMap.map(row[static_cast<std::size_t>(c)]);
         const int offset = c * 4;
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

   const float norm = (db - _minDb) / (_maxDb - _minDb);
   return std::clamp(norm, 0.0F, 1.0F);
}

void WaterfallWidget::drawFrequencyLabels(QPainter& painter, const QRect& area) const
{
   painter.setPen(QColor(180, 180, 190));
   QFont font = painter.font();
   font.setPointSize(8);
   painter.setFont(font);

   constexpr int X_TICKS = 8;
   const bool hasFreq = (_bandwidthHz > 0.0);
   const double startFreq = _centerFreqHz - (_bandwidthHz / 2.0);

   for (int i = 0; i <= X_TICKS; ++i)
   {
      const float frac = static_cast<float>(i) / static_cast<float>(X_TICKS);
      const int xPos = area.left() + static_cast<int>(frac * static_cast<float>(area.width()));

      QString label;
      if (hasFreq)
      {
         const double freq = startFreq + (static_cast<double>(frac) * _bandwidthHz);
         label = QString::fromStdString(formatFrequency(freq));
      }
      else
      {
         label = QString::number(i);
      }

      constexpr int LABEL_WIDTH = 80;
      painter.drawText(xPos - (LABEL_WIDTH / 2), area.bottom() + 3,
                       LABEL_WIDTH, MARGIN_BOTTOM - 5,
                       Qt::AlignCenter, label);
   }
}

} // namespace RealTimeGraphs
