#include "RealTimeGraphs/SpectrumWidget.h"

#include "RealTimeGraphs/ColorBarWidget.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>

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

   _colorBar = new ColorBarStrip(this);
   _colorBar->setDbRange(_minDb, _maxDb);
   _colorBar->setColorMap(_colorMap);
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
   _viewMinDb = static_cast<double>(minDb);
   _viewMaxDb = static_cast<double>(maxDb);
   syncColorBar();
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
   syncColorBar();
   update();
}

void SpectrumWidget::setGridLines(int count)
{
   _gridLines = count;
   update();
}

void SpectrumWidget::setFrequencyRange(double centerFreqHz, double bandwidthHz)
{
   _centerFreqHz = centerFreqHz;
   _bandwidthHz  = bandwidthHz;
   _viewXStart   = 0.0;
   _viewXEnd     = 1.0;
   update();
}

QSize SpectrumWidget::minimumSizeHint() const
{
   return {320, 200};
}

void SpectrumWidget::resetView()
{
   _viewMinDb  = static_cast<double>(_minDb);
   _viewMaxDb  = static_cast<double>(_maxDb);
   _viewXStart = 0.0;
   _viewXEnd   = 1.0;
   syncColorBar();
   update();
}

void SpectrumWidget::setColorBarVisible(bool visible)
{
   _colorBar->setVisible(visible);
   update();
}

QRect SpectrumWidget::plotArea() const
{
   return {MARGIN_LEFT, MARGIN_TOP,
           width() - MARGIN_LEFT - MARGIN_RIGHT,
           height() - MARGIN_TOP - MARGIN_BOTTOM};
}

// ============================================================================
// Paint
// ============================================================================

void SpectrumWidget::resizeEvent(QResizeEvent* event)
{
   QWidget::resizeEvent(event);
   QRect area = plotArea();
   _colorBar->setGeometry(
      area.right() + 5,
      area.top(),
      COLOR_BAR_WIDTH,
      area.height());
}

void SpectrumWidget::paintEvent(QPaintEvent* /*event*/)
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, false);

   QRect area = plotArea();

   drawBackground(painter, area);
   drawGrid(painter, area);

   // Clip spectrum to the plot area so zoomed/panned data
   // does not overflow into the label margins.
   painter.save();
   painter.setClipRect(area);
   drawSpectrum(painter, area);
   painter.restore();

   drawLabels(painter, area);
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

void SpectrumWidget::drawSpectrum(QPainter& painter, const QRect& area)
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
   double viewWidth = _viewXEnd - _viewXStart;
   if (viewWidth <= 0.0)
   {
      viewWidth = 1.0;
   }

   // Determine visible bin range (with 1-bin margin for line continuity)
   int firstBin = std::max(0,
      static_cast<int>(std::floor(_viewXStart * static_cast<double>(binCount))) - 1);
   int lastBin = std::min(binCount - 1,
      static_cast<int>(std::ceil(_viewXEnd * static_cast<double>(binCount))));

   // Pre-compute normalised values and positions for visible bins
   std::vector<float> norms(snapshot.size());
   std::vector<float> xPts(snapshot.size());
   std::vector<float> yPts(snapshot.size());

   for (int i = firstBin; i <= lastBin; ++i)
   {
      auto si = static_cast<std::size_t>(i);
      float norm = toNormalised(snapshot[si]);
      norms[si] = norm;

      double binFrac = (static_cast<double>(i) + 0.5) / static_cast<double>(binCount);
      double screenFrac = (binFrac - _viewXStart) / viewWidth;

      xPts[si] = static_cast<float>(area.left()) +
                 static_cast<float>(screenFrac) * static_cast<float>(area.width());
      yPts[si] = static_cast<float>(area.bottom()) -
                 norm * static_cast<float>(area.height());
   }

   // Build the fill path
   QPainterPath fillPath;
   fillPath.moveTo(static_cast<double>(xPts[static_cast<std::size_t>(firstBin)]),
                   static_cast<double>(area.bottom()));
   for (int i = firstBin; i <= lastBin; ++i)
   {
      fillPath.lineTo(static_cast<double>(xPts[static_cast<std::size_t>(i)]),
                      static_cast<double>(yPts[static_cast<std::size_t>(i)]));
   }
   fillPath.lineTo(static_cast<double>(xPts[static_cast<std::size_t>(lastBin)]),
                   static_cast<double>(area.bottom()));
   fillPath.closeSubpath();

   // Draw gradient fill under the curve matching the colour bar
   QLinearGradient gradient(0, area.top(), 0, area.bottom());
   constexpr int GRADIENT_STOPS = 16;
   for (int s = 0; s <= GRADIENT_STOPS; ++s)
   {
      float frac = static_cast<float>(s) / static_cast<float>(GRADIENT_STOPS);
      // frac 0 = top of plot = high value, frac 1 = bottom = low value
      float norm = 1.0F - frac;
      Color c = _colorMap.map(norm);
      gradient.setColorAt(static_cast<double>(frac),
                          QColor(c.r, c.g, c.b, 70));
   }

   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.fillPath(fillPath, gradient);

   // Draw per-segment coloured line on top — each segment uses the
   // average normalised value of its two endpoints to pick a colour.
   for (int i = firstBin + 1; i <= lastBin; ++i)
   {
      auto si = static_cast<std::size_t>(i);
      float avgNorm = (norms[si - 1] + norms[si]) * 0.5F;
      Color c = _colorMap.map(avgNorm);
      painter.setPen(QPen(QColor(c.r, c.g, c.b, 220), 1.5));
      painter.drawLine(QPointF(static_cast<double>(xPts[si - 1]),
                               static_cast<double>(yPts[si - 1])),
                       QPointF(static_cast<double>(xPts[si]),
                               static_cast<double>(yPts[si])));
   }
   painter.setRenderHint(QPainter::Antialiasing, false);
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
      float db = static_cast<float>(_viewMaxDb -
                    static_cast<double>(frac) * (_viewMaxDb - _viewMinDb));
      int yPos = area.top() + static_cast<int>(frac * static_cast<float>(area.height()));

      QString label = QString::number(static_cast<int>(db)) + " dB";
      painter.drawText(0, yPos - 6, MARGIN_LEFT - 5, 12,
                       Qt::AlignRight | Qt::AlignVCenter, label);
   }

   // X-axis: frequency tick labels
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
         double dataFrac = _viewXStart +
                           static_cast<double>(frac) * (_viewXEnd - _viewXStart);
         double freq = startFreq + dataFrac * _bandwidthHz;
         label = QString::fromStdString(formatFrequency(freq));
      }
      else
      {
         double dataFrac = _viewXStart +
                           static_cast<double>(frac) * (_viewXEnd - _viewXStart);
         label = QString::number(dataFrac, 'f', 2);
      }

      // Centre the label on the tick position
      constexpr int LABEL_WIDTH = 80;
      painter.drawText(xPos - LABEL_WIDTH / 2, area.bottom() + 3,
                       LABEL_WIDTH, MARGIN_BOTTOM - 5,
                       Qt::AlignCenter, label);
   }
}

// ============================================================================
// Mouse interaction
// ============================================================================

void SpectrumWidget::wheelEvent(QWheelEvent* event)
{
   QRect area = plotArea();
   QPointF pos = event->position();

   // Determine which axis region the cursor is in:
   //   Y-axis margin (left of plot), X-axis margin (below plot), or plot area.
   //   The colour-bar area (right of plot) also counts as Y-axis.
   bool inPlot   = area.contains(pos.toPoint());
   bool inYMargin = ((pos.x() < area.left() || pos.x() > area.right()) &&
                     pos.y() >= area.top() && pos.y() <= area.bottom());
   bool inXMargin = (pos.y() > area.bottom() &&
                     pos.x() >= area.left() && pos.x() <= area.right());

   if (!inPlot && !inYMargin && !inXMargin)
   {
      QWidget::wheelEvent(event);
      return;
   }

   constexpr double ZOOM_FACTOR = 1.15;
   double factor = (event->angleDelta().y() > 0)
                      ? (1.0 / ZOOM_FACTOR)
                      : ZOOM_FACTOR;

   bool zoomY = inPlot || inYMargin;
   bool zoomX = inPlot || inXMargin;

   if (zoomY)
   {
      // Y-axis zoom centred on the dB value under the mouse
      double yFrac = (pos.y() - area.top()) / area.height();
      yFrac = std::clamp(yFrac, 0.0, 1.0);
      double dbAtMouse = _viewMaxDb - yFrac * (_viewMaxDb - _viewMinDb);
      _viewMinDb = dbAtMouse - (dbAtMouse - _viewMinDb) * factor;
      _viewMaxDb = dbAtMouse + (_viewMaxDb - dbAtMouse) * factor;
   }

   if (zoomX)
   {
      // X-axis zoom centred on the data fraction under the mouse
      double xFrac = (pos.x() - area.left()) / area.width();
      xFrac = std::clamp(xFrac, 0.0, 1.0);
      double dataFracAtMouse = _viewXStart + xFrac * (_viewXEnd - _viewXStart);
      double newXStart = dataFracAtMouse - (dataFracAtMouse - _viewXStart) * factor;
      double newXEnd   = dataFracAtMouse + (_viewXEnd - dataFracAtMouse) * factor;
      _viewXStart = std::max(0.0, newXStart);
      _viewXEnd   = std::min(1.0, newXEnd);
   }

   syncColorBar();
   update();
   event->accept();
}

void SpectrumWidget::mousePressEvent(QMouseEvent* event)
{
   if (event->button() == Qt::LeftButton)
   {
      QRect area = plotArea();
      QPoint pos = event->pos();

      bool inPlot   = area.contains(pos);
      bool inYMargin = ((pos.x() < area.left() || pos.x() > area.right()) &&
                        pos.y() >= area.top() && pos.y() <= area.bottom());
      bool inXMargin = (pos.y() > area.bottom() &&
                        pos.x() >= area.left() && pos.x() <= area.right());

      if (inPlot || inYMargin || inXMargin)
      {
         _panning        = true;
         _panStartPos    = pos;
         _panStartMinDb  = _viewMinDb;
         _panStartMaxDb  = _viewMaxDb;
         _panStartXStart = _viewXStart;
         _panStartXEnd   = _viewXEnd;

         if (inYMargin)
         {
            _panAxis = PanAxis::YOnly;
         }
         else if (inXMargin)
         {
            _panAxis = PanAxis::XOnly;
         }
         else
         {
            _panAxis = PanAxis::Both;
         }

         setCursor(Qt::ClosedHandCursor);
         event->accept();
         return;
      }
   }
   QWidget::mousePressEvent(event);
}

void SpectrumWidget::mouseMoveEvent(QMouseEvent* event)
{
   if (_panning)
   {
      QRect area = plotArea();
      double dxPixels = event->pos().x() - _panStartPos.x();
      double dyPixels = event->pos().y() - _panStartPos.y();

      bool panX = (_panAxis == PanAxis::Both || _panAxis == PanAxis::XOnly);
      bool panY = (_panAxis == PanAxis::Both || _panAxis == PanAxis::YOnly);

      if (panX)
      {
         // X pan: drag right -> content moves right -> view shifts left
         double xRange = _panStartXEnd - _panStartXStart;
         double dxData = -dxPixels / area.width() * xRange;
         double newXStart = _panStartXStart + dxData;
         double newXEnd   = _panStartXEnd   + dxData;

         if (newXStart < 0.0)
         {
            newXEnd  -= newXStart;
            newXStart = 0.0;
         }
         if (newXEnd > 1.0)
         {
            newXStart -= (newXEnd - 1.0);
            newXEnd    = 1.0;
         }
         _viewXStart = std::max(0.0, newXStart);
         _viewXEnd   = std::min(1.0, newXEnd);
      }

      if (panY)
      {
         // Y pan: drag down -> content moves down -> view shifts up in dB
         double dbRange = _panStartMaxDb - _panStartMinDb;
         double dyDb    = dyPixels / area.height() * dbRange;
         _viewMinDb = _panStartMinDb + dyDb;
         _viewMaxDb = _panStartMaxDb + dyDb;
      }

      syncColorBar();
      update();
      event->accept();
   }
   else
   {
      QWidget::mouseMoveEvent(event);
   }
}

void SpectrumWidget::mouseReleaseEvent(QMouseEvent* event)
{
   if (event->button() == Qt::LeftButton && _panning)
   {
      _panning = false;
      setCursor(Qt::ArrowCursor);
      event->accept();
   }
   else
   {
      QWidget::mouseReleaseEvent(event);
   }
}

void SpectrumWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
   if (event->button() == Qt::LeftButton)
   {
      resetView();
      event->accept();
   }
   else
   {
      QWidget::mouseDoubleClickEvent(event);
   }
}

// ============================================================================
// Utilities
// ============================================================================

void SpectrumWidget::syncColorBar()
{
   _colorBar->setDbRange(static_cast<float>(_viewMinDb),
                         static_cast<float>(_viewMaxDb));
   _colorBar->setColorMap(_colorMap);
}

float SpectrumWidget::toNormalised(float value) const
{
   float db = value;
   if (!_inputIsDb)
   {
      // Convert linear magnitude to dB
      constexpr float EPSILON = 1.0e-10F;
      db = 20.0F * std::log10(std::max(value, EPSILON));
   }

   // Map dB range to [0, 1] using the current view range
   auto viewMin = static_cast<float>(_viewMinDb);
   auto viewMax = static_cast<float>(_viewMaxDb);
   float norm = (db - viewMin) / (viewMax - viewMin);
   return std::clamp(norm, 0.0F, 1.0F);
}

std::string SpectrumWidget::formatFrequency(double freqHz)
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
