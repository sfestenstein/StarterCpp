#include "RealTimeGraphs/ConstellationWidget.h"

#include <QPainter>
#include <QPaintEvent>

#include <algorithm>
#include <cmath>

namespace RealTimeGraphs
{

// ============================================================================
// Construction
// ============================================================================

ConstellationWidget::ConstellationWidget(int historySize, QWidget* parent)
   : QWidget(parent)
   , _points(static_cast<std::size_t>(historySize))
{
   setMinimumSize(minimumSizeHint());
   setAttribute(Qt::WA_OpaquePaintEvent);
}

// ============================================================================
// Public API
// ============================================================================

void ConstellationWidget::setData(const std::vector<std::complex<float>>& samples)
{
   {
      std::lock_guard<std::mutex> lock(_mutex);
      for (const auto& s : samples)
      {
         _points.push(s);
      }
   }
   update();
}

void ConstellationWidget::setAxisRange(float range)
{
   _axisRange = range;
   update();
}

void ConstellationWidget::setPointSize(int size)
{
   _pointSize = size;
   update();
}

void ConstellationWidget::setPersistence(bool enable)
{
   _persistence = enable;
   update();
}

void ConstellationWidget::setPersistenceDepth(int depth)
{
   std::lock_guard<std::mutex> lock(_mutex);
   _points = CommonUtils::CircularBuffer<std::complex<float>>(static_cast<std::size_t>(depth));
   update();
}

void ConstellationWidget::setDotColor(const QColor& color)
{
   _dotColor = color;
   update();
}

void ConstellationWidget::setGridEnabled(bool enable)
{
   _gridEnabled = enable;
   update();
}

QSize ConstellationWidget::minimumSizeHint() const
{
   return {300, 300};
}

// ============================================================================
// Paint
// ============================================================================

void ConstellationWidget::paintEvent(QPaintEvent* /*event*/)
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, true);

   // Keep the plot area square
   int side = std::min(width() - MARGIN_LEFT - MARGIN_RIGHT,
                       height() - MARGIN_TOP - MARGIN_BOTTOM);
   if (side <= 0)
   {
      return;
   }

   int xOffset = MARGIN_LEFT + (width() - MARGIN_LEFT - MARGIN_RIGHT - side) / 2;
   int yOffset = MARGIN_TOP + (height() - MARGIN_TOP - MARGIN_BOTTOM - side) / 2;
   QRect plotArea(xOffset, yOffset, side, side);

   drawBackground(painter, plotArea);
   if (_gridEnabled)
   {
      drawGrid(painter, plotArea);
   }
   drawPoints(painter, plotArea);

   // Axis labels
   painter.setPen(QColor(180, 180, 190));
   QFont font = painter.font();
   font.setPointSize(8);
   painter.setFont(font);

   painter.drawText(plotArea.left(), plotArea.bottom() + 5,
                    plotArea.width(), MARGIN_BOTTOM - 5,
                    Qt::AlignCenter, "In-Phase (I)");

   painter.save();
   painter.translate(12, plotArea.center().y());
   painter.rotate(-90);
   painter.drawText(-plotArea.height() / 2, 0, plotArea.height(), 15,
                    Qt::AlignCenter, "Quadrature (Q)");
   painter.restore();
}

// ============================================================================
// Drawing helpers
// ============================================================================

void ConstellationWidget::drawBackground(QPainter& painter, const QRect& area)
{
   painter.fillRect(rect(), QColor(25, 25, 30));
   painter.fillRect(area, QColor(10, 10, 15));
}

void ConstellationWidget::drawGrid(QPainter& painter, const QRect& area)
{
   // Draw concentric circles at 0.25, 0.5, 0.75, 1.0 of axis range
   painter.setPen(QPen(QColor(50, 50, 60), 1, Qt::DotLine));

   QPoint centre = area.center();

   for (int i = 1; i <= 4; ++i)
   {
      float frac = static_cast<float>(i) / 4.0F;
      float radius = frac * static_cast<float>(area.width()) / 2.0F;
      int r = static_cast<int>(radius);
      painter.drawEllipse(centre, r, r);
   }

   // Draw cross-hairs (axes)
   painter.setPen(QPen(QColor(70, 70, 80), 1, Qt::SolidLine));
   painter.drawLine(area.left(), centre.y(), area.right(), centre.y());
   painter.drawLine(centre.x(), area.top(), centre.x(), area.bottom());

   // Draw axis tick labels
   painter.setPen(QColor(140, 140, 150));
   QFont font = painter.font();
   font.setPointSize(7);
   painter.setFont(font);

   for (int i = -2; i <= 2; ++i)
   {
      if (i == 0) continue;

      float val = _axisRange * static_cast<float>(i) / 2.0F;
      QString label = QString::number(static_cast<double>(val), 'g', 2);

      // X-axis ticks
      QPoint px = mapToPixel(val, 0.0F, area);
      painter.drawText(px.x() - 15, area.bottom() + 2, 30, 12, Qt::AlignCenter, label);

      // Y-axis ticks
      QPoint py = mapToPixel(0.0F, val, area);
      painter.drawText(area.left() - 35, py.y() - 6, 30, 12,
                       Qt::AlignRight | Qt::AlignVCenter, label);
   }
}

void ConstellationWidget::drawPoints(QPainter& painter, const QRect& area)
{
   std::lock_guard<std::mutex> lock(_mutex);

   auto count = _points.size();
   if (count == 0)
   {
      return;
   }

   painter.setPen(Qt::NoPen);

   for (std::size_t i = 0; i < count; ++i)
   {
      const auto& sample = _points[i];
      QPoint pos = mapToPixel(sample.real(), sample.imag(), area);

      // Check if point is within plot area
      if (!area.contains(pos))
      {
         continue;
      }

      if (_persistence)
      {
         // Fade older points: alpha ramps from 20 (oldest) to 255 (newest)
         float ageFrac = static_cast<float>(i) / static_cast<float>(count);
         int alpha = 20 + static_cast<int>(ageFrac * 235.0F);
         painter.setBrush(QColor(_dotColor.red(), _dotColor.green(),
                                 _dotColor.blue(), alpha));
      }
      else
      {
         painter.setBrush(_dotColor);
      }

      painter.drawEllipse(pos, _pointSize, _pointSize);
   }
}

QPoint ConstellationWidget::mapToPixel(float i, float q, const QRect& area) const
{
   // Map I/Q value from [-_axisRange, +_axisRange] to pixel coordinates
   float normX = (i + _axisRange) / (2.0F * _axisRange);
   float normY = 1.0F - (q + _axisRange) / (2.0F * _axisRange); // Y is inverted

   int px = area.left() + static_cast<int>(normX * static_cast<float>(area.width()));
   int py = area.top() + static_cast<int>(normY * static_cast<float>(area.height()));
   return {px, py};
}

} // namespace RealTimeGraphs
