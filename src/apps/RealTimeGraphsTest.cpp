// =============================================================================
// RealTimeGraphsTest — Interactive demo of the RealTimeGraphs widget library.
//
// Three tabs:
//   1. Spectrum   — swept-tone FFT bar chart
//   2. Waterfall  — same data rendered as a scrolling spectrogram
//   3. Constellation — I/Q scatter of a rotating QPSK-like pattern + noise
//
// All data is generated internally from synthetic waveforms.
// =============================================================================

// Project headers
#include "CommonUtils/GeneralLogger.h"
#include "RealTimeGraphs/ColorBarWidget.h"
#include "RealTimeGraphs/ColorMap.h"
#include "RealTimeGraphs/ConstellationWidget.h"
#include "RealTimeGraphs/SpectrumWidget.h"
#include "RealTimeGraphs/WaterfallWidget.h"

// Third-party headers
#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// System headers
#include <cmath>
#include <complex>
#include <cstdlib>
#include <numbers>
#include <random>
#include <vector>

namespace
{

// ============================================================================
// Synthetic waveform generators
// ============================================================================

constexpr int FFT_BINS = 256;
constexpr float TWO_PI = 2.0F * std::numbers::pi_v<float>;

/// Generate a fake magnitude spectrum with a tone sweeping across the bins.
std::vector<float> generateSpectrum(int frame)
{
   std::vector<float> mag(FFT_BINS, 0.0F);

   // Sweep a main peak across the spectrum
   float centre = static_cast<float>(frame % FFT_BINS);
   constexpr float SIGMA = 8.0F;

   for (int i = 0; i < FFT_BINS; ++i)
   {
      float dist = static_cast<float>(i) - centre;
      float peak = std::exp(-0.5F * (dist * dist) / (SIGMA * SIGMA));

      // Add a second harmonic at double the frequency
      float centre2 = std::fmod(centre * 2.0F, static_cast<float>(FFT_BINS));
      float dist2 = static_cast<float>(i) - centre2;
      float harmonic = 0.3F * std::exp(-0.5F * (dist2 * dist2) / (SIGMA * SIGMA));

      // Noise floor
      float noise = 0.005F + 0.003F * (static_cast<float>(std::rand()) /
                                        static_cast<float>(RAND_MAX));

      mag[static_cast<std::size_t>(i)] = peak + harmonic + noise;
   }
   return mag;
}

/// Generate a batch of noisy QPSK-like constellation points with phase rotation.
std::vector<std::complex<float>> generateConstellation(int frame, int count)
{
   static std::mt19937 gen(42); // NOLINT(cert-msc32-c,cert-msc51-cpp)
   static std::normal_distribution<float> noise(0.0F, 0.08F);

   std::vector<std::complex<float>> pts;
   pts.reserve(static_cast<std::size_t>(count));

   // Slowly rotating reference phase
   float phaseOffset = static_cast<float>(frame) * 0.02F;

   for (int i = 0; i < count; ++i)
   {
      // Pick one of four QPSK symbol points
      int symbol = i % 4;
      float angle = TWO_PI * (static_cast<float>(symbol) + 0.5F) / 4.0F + phaseOffset;
      constexpr float RADIUS = 0.7F;

      float real = RADIUS * std::cos(angle) + noise(gen);
      float imag = RADIUS * std::sin(angle) + noise(gen);
      pts.emplace_back(real, imag);
   }
   return pts;
}

} // anonymous namespace

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[])
{
   CommonUtils::GeneralLogger logger;
   logger.init("RealTimeGraphsTest");
   GPINFO("Starting RealTimeGraphsTest demo application");

   QApplication app(argc, argv);
   app.setApplicationName("RealTimeGraphsTest");

   // ---- Create widgets ----
   auto* spectrum      = new RealTimeGraphs::SpectrumWidget;
   auto* waterfall     = new RealTimeGraphs::WaterfallWidget(256);
   auto* constellation = new RealTimeGraphs::ConstellationWidget(8192);

   auto* spectrumBar  = new RealTimeGraphs::ColorBarWidget;
   auto* waterfallBar = new RealTimeGraphs::ColorBarWidget;

   spectrum->setDbRange(-100.0F, 0.0F);
   waterfall->setDbRange(-100.0F, 0.0F);
   spectrumBar->setDbRange(-100.0F, 0.0F);
   waterfallBar->setDbRange(-100.0F, 0.0F);
   constellation->setAxisRange(1.5F);

   // Set a representative frequency range: 2.45 GHz centre, 20 MHz bandwidth
   spectrum->setFrequencyRange(2.45e9, 20.0e6);
   waterfall->setFrequencyRange(2.45e9, 20.0e6);

   // ---- Build per-tab layouts with colour-map selectors ----

   auto buildTabPage = [](QWidget* graphWidget,
                          RealTimeGraphs::ColorBarWidget* colorBar,
                          const QString& label,
                          auto onPaletteChanged) -> QWidget*
   {
      auto* page = new QWidget;
      auto* layout = new QVBoxLayout(page);

      // Toolbar row
      auto* toolbar = new QHBoxLayout;
      toolbar->addWidget(new QLabel("Color Map:"));

      auto* combo = new QComboBox;
      for (std::size_t i = 0; i < RealTimeGraphs::ColorMap::paletteCount(); ++i)
      {
         auto pal = RealTimeGraphs::ColorMap::paletteAt(i);
         combo->addItem(
            QString::fromStdString(RealTimeGraphs::ColorMap::paletteName(pal)),
            static_cast<int>(i));
      }
      QObject::connect(combo, &QComboBox::currentIndexChanged,
                       [onPaletteChanged](int idx)
                       {
                          auto pal = RealTimeGraphs::ColorMap::paletteAt(
                             static_cast<std::size_t>(idx));
                          onPaletteChanged(pal);
                       });
      toolbar->addWidget(combo);
      toolbar->addWidget(new QLabel(label));
      toolbar->addStretch();

      layout->addLayout(toolbar);

      // Plot + colour bar side by side
      auto* plotRow = new QHBoxLayout;
      plotRow->addWidget(graphWidget, 1);
      plotRow->addWidget(colorBar, 0);
      layout->addLayout(plotRow, 1);
      return page;
   };

   QWidget* spectrumPage = buildTabPage(spectrum, spectrumBar, "Swept-tone FFT",
      [spectrum, spectrumBar](RealTimeGraphs::ColorMap::Palette p)
      {
         spectrum->setColorMap(p);
         spectrumBar->setColorMap(p);
      });

   QObject::connect(spectrumBar, &RealTimeGraphs::ColorBarWidget::dbRangeChanged,
                    [spectrum](float minDb, float maxDb)
                    {
                       spectrum->setDbRange(minDb, maxDb);
                    });

   QWidget* waterfallPage = buildTabPage(waterfall, waterfallBar, "Spectrogram",
      [waterfall, waterfallBar](RealTimeGraphs::ColorMap::Palette p)
      {
         waterfall->setColorMap(p);
         waterfallBar->setColorMap(p);
      });

   QObject::connect(waterfallBar, &RealTimeGraphs::ColorBarWidget::dbRangeChanged,
                    [waterfall](float minDb, float maxDb)
                    {
                       waterfall->setDbRange(minDb, maxDb);
                    });

   // Constellation doesn't use a colour map, but we keep the layout consistent
   auto* constPage = new QWidget;
   auto* constLayout = new QVBoxLayout(constPage);
   auto* constToolbar = new QHBoxLayout;
   constToolbar->addWidget(new QLabel("I/Q Constellation — rotating QPSK + noise"));
   constToolbar->addStretch();
   constLayout->addLayout(constToolbar);
   constLayout->addWidget(constellation, 1);

   // ---- Tab widget ----
   auto* tabs = new QTabWidget;
   tabs->addTab(spectrumPage, "Spectrum");
   tabs->addTab(waterfallPage, "Waterfall");
   tabs->addTab(constPage, "Constellation");

   // ---- Main window ----
   auto* central = new QWidget;
   auto* mainLayout = new QVBoxLayout(central);
   mainLayout->setContentsMargins(4, 4, 4, 4);
   mainLayout->addWidget(tabs);

   QMainWindow window;
   window.setWindowTitle("RealTimeGraphs Test");
   window.setCentralWidget(central);
   window.resize(900, 600);
   window.show();

   // ---- Timer-driven data feed at ~30 FPS ----
   int frame = 0;
   constexpr int TIMER_INTERVAL_MS = 33; // ~30 FPS

   QTimer timer;
   QObject::connect(&timer, &QTimer::timeout, [&]()
   {
      // Spectrum & waterfall share the same data
      auto mag = generateSpectrum(frame);
      spectrum->setData(mag);
      waterfall->addRow(mag);

      // Constellation gets a fresh batch of symbols
      constexpr int SYMBOLS_PER_FRAME = 64;
      auto iq = generateConstellation(frame, SYMBOLS_PER_FRAME);
      constellation->setData(iq);

      ++frame;
   });
   timer.start(TIMER_INTERVAL_MS);
   GPINFO("Data feed running at {} FPS ({} ms interval)",
          1000 / TIMER_INTERVAL_MS, TIMER_INTERVAL_MS);

   int result = app.exec();
   GPINFO("Application exited with code {}", result);
   return result;
}
