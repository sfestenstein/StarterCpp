#include "RealTimeGraphs/SpectrumWidget.h"

#include <QPainter>
#include <QPaintEvent>

#include <algorithm>
#include <cmath>

namespace RealTimeGraphs
{

// ============================================================================
// Construction
// ============================================================================

SpectrumWidget::SpectrumWidget(QWidget* parent)
   : QWidget(parent)
{
   setMinimumSize(minimumSizeHint());
   setAttribute(Qt::WA_OpaquePaintEvent);
}

// ============================================================================
// Public API
// ============================================================================

void SpectrumWidget::setData(const std::vector<float>& magnitudes)
{
   {
      std::lock_guard<std::mutex> lock(_mutex);
      _data = magnitudes;
   }
   update(); // schedule repaint on the GUI thread
}

void SpectrumWidget::setDbRange(float minDb, float maxDb)
{
   _minDb = minDb;
   _maxDb = maxDb;
   update();
}

void SpectrumWidget::setInputIsDb(bool isDb)
{
   _inputIsDb = isDb;
   update();
}

void SpectrumWidget::setColorMap(ColorMap::Palette palette)
{
   _colorMap = ColorMap(palette);
   update();
}

void SpectrumWidget::setGridLines(int count)
{
   _gridLines = count;
   update();
}

QSize SpectrumWidget::minimumSizeHint() const
{
   return {320, 200};
}

// ============================================================================
// Paint
// ============================================================================

void SpectrumWidget::paintEvent(QPaintEvent* /*event*/)
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, false);

   QRect plotArea(MARGIN_LEFT, MARGIN_TOP,
                  width() - MARGIN_LEFT - MARGIN_RIGHT,
                  height() - MARGIN_TOP - MARGIN_BOTTOM);

   drawBackground(painter, plotArea);
   drawGrid(painter, plotArea);
   drawBars(painter, plotArea);
   drawLabels(painter, plotArea);
}

// ============================================================================
// Drawing helpers
// ============================================================================

void SpectrumWidget::drawBackground(QPainter& painter, const QRect& area)
{
   painter.fillRect(rect(), QColor(25, 25, 30));
   painter.fillRect(area, QColor(15, 15, 20));
}

void SpectrumWidget::drawGrid(QPainter& painter, const QRect& area)
{
   painter.setPen(QPen(QColor(60, 60, 70), 1, Qt::DotLine));

   // Horizontal grid lines (amplitude)
   for (int i = 0; i <= _gridLines; ++i)
   {
      float frac = static_cast<float>(i) / static_cast<float>(_gridLines);
      int yPos = area.top() + static_cast<int>(frac * static_cast<float>(area.height()));
      painter.drawLine(area.left(), yPos, area.right(), yPos);
   }

   // Vertical grid lines (bins / frequency)
   constexpr int V_LINES = 8;
   for (int i = 0; i <= V_LINES; ++i)
   {
      float frac = static_cast<float>(i) / static_cast<float>(V_LINES);
      int xPos = area.left() + static_cast<int>(frac * static_cast<float>(area.width()));
      painter.drawLine(xPos, area.top(), xPos, area.bottom());
   }
}

void SpectrumWidget::drawBars(QPainter& painter, const QRect& area)
{
   std::vector<float> snapshot;
   {
      std::lock_guard<std::mutex> lock(_mutex);
      snapshot = _data;
   }

   if (snapshot.empty())
   {
      return;
   }

   auto binCount = static_cast<int>(snapshot.size());
   float barWidth = static_cast<float>(area.width()) / static_cast<float>(binCount);

   for (int i = 0; i < binCount; ++i)
   {
      float norm = toNormalised(snapshot[static_cast<std::size_t>(i)]);
      float barHeight = norm * static_cast<float>(area.height());

      int xPos = area.left() + static_cast<int>(static_cast<float>(i) * barWidth);
      int yPos = area.bottom() - static_cast<int>(barHeight);
      int wid  = std::max(1, static_cast<int>(barWidth));
      int hgt  = static_cast<int>(barHeight);

      Color c = _colorMap.map(norm);
      painter.fillRect(xPos, yPos, wid, hgt, QColor(c.r, c.g, c.b, c.a));
   }
}

void SpectrumWidget::drawLabels(QPainter& painter, const QRect& area)
{
   painter.setPen(QColor(180, 180, 190));
   QFont font = painter.font();
   font.setPointSize(8);
   painter.setFont(font);

   // Y-axis dB labels
   for (int i = 0; i <= _gridLines; ++i)
   {
      float frac = static_cast<float>(i) / static_cast<float>(_gridLines);
      float db = _maxDb - frac * (_maxDb - _minDb);
      int yPos = area.top() + static_cast<int>(frac * static_cast<float>(area.height()));

      QString label = QString::number(static_cast<int>(db)) + " dB";
      painter.drawText(0, yPos - 6, MARGIN_LEFT - 5, 12, Qt::AlignRight | Qt::AlignVCenter, label);
   }

   // X-axis label
   painter.drawText(area.left(), area.bottom() + 5,
                    area.width(), MARGIN_BOTTOM - 5,
                    Qt::AlignCenter, "Frequency Bin");
}

// ============================================================================
// Utilities
// ============================================================================

float SpectrumWidget::toNormalised(float value) const
{
   float db = value;
   if (!_inputIsDb)
   {
      // Convert linear magnitude to dB
      constexpr float EPSILON = 1.0e-10F;
      db = 20.0F * std::log10(std::max(value, EPSILON));
   }

   // Map dB range to [0, 1]
   float norm = (db - _minDb) / (_maxDb - _minDb);
   return std::clamp(norm, 0.0F, 1.0F);
}

} // namespace RealTimeGraphs
