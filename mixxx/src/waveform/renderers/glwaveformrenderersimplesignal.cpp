#include "glwaveformrenderersimplesignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "waveform/waveformwidgetfactory.h"

#include <qgl.h>

GLWaveformRendererSimpleSignal::GLWaveformRendererSimpleSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererSimpleSignal::~GLWaveformRendererSimpleSignal(){
}

void GLWaveformRendererSimpleSignal::onInit() {
}

void GLWaveformRendererSimpleSignal::onSetup(const QDomNode &node){
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

void GLWaveformRendererSimpleSignal::draw(QPainter* painter, QPaintEvent* /*event*/){

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    const Waveform* waveform = pTrack->getWaveform();
    if (waveform == NULL) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    const int firstIndex = int(firstVisualIndex+0.5);
    firstVisualIndex = firstIndex - firstIndex%2;

    const int lastIndex = int(lastVisualIndex+0.5);
    lastVisualIndex = lastIndex + lastIndex%2;

    // save the GL state set for QPainter
    painter->beginNativePainting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(1.1);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE_ARB);

    const QColor& color = m_colors.getSignalColor();

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    double visualGain = factory->getVisualGain(::WaveformWidgetFactory::All);

    float maxAll[2];

    glPushMatrix();

    if( m_alignment == Qt::AlignCenter) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1.f,2.f*visualGain*m_waveformRenderer->getGain(),1.f);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if( visualIndex < 0)
                    continue;

                if( visualIndex > dataSize - 1)
                    break;

                maxAll[0] = (float)data[visualIndex].filtered.all;
                maxAll[1] = (float)data[visualIndex+1].filtered.all;
                glColor4f(color.redF(),color.greenF(),color.blueF(),0.9);
                glVertex2f(visualIndex,maxAll[0]);
                glVertex2f(visualIndex,-1.f*maxAll[1]);
            }
        }
        glEnd();
    } else { //top || bottom
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if( m_alignment == Qt::AlignBottom)
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        else
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1.f,visualGain*m_waveformRenderer->getGain(),1.f);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if( visualIndex < 0)
                    continue;

                if( visualIndex > dataSize - 1)
                    break;

                maxAll[0] = (float)data[visualIndex].filtered.all;
                maxAll[1] = (float)data[visualIndex+1].filtered.all;
                glColor4f(color.redF(),color.greenF(),color.blueF(),0.8);
                glVertex2f(float(visualIndex),0.f);
                glVertex2f(float(visualIndex),math_max(maxAll[0],maxAll[1]));
            }
        }
        glEnd();
    }

    glPopMatrix();

    glDisable(GL_BLEND);

    painter->endNativePainting();
}


