/*
  Copyright (C) 2015 Paul Brossier <piem@aubio.org>

  This file is part of ofxAubio.

  ofxAubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ofxAubioAttackClass.h"
#include "ofLog.h"

ofEvent<int> ofxAubioAttackClass::gotGlobalOnsetClass = ofEvent<int>();
ofEvent<int> ofxAubioAttackClass::gotGlobalBeatClass = ofEvent<int>();

ofxAubioAttackClass::ofxAubioAttackClass()
    : bands(NULL)
{
    lag_onset = 3;
    lag_beat = 3;
}

ofxAubioAttackClass::~ofxAubioAttackClass()
{
}

void ofxAubioAttackClass::setup()
{
    setup("default", 512, 256, 44100);
}
void ofxAubioAttackClass::setup(string method, int buf_s, int hop_s, int samplerate)
{
    //setup("default", 512, 256, 44100);
}

void ofxAubioAttackClass::setBands(ofxAubioMelBands & _bands) {
    bands = &_bands;
}

void ofxAubioAttackClass::setOnset(ofxAubioOnset & _onset) {
    onset = &_onset;
    ofAddListener(onset->gotOnset, this, &ofxAubioAttackClass::onsetEvent);
}

void ofxAubioAttackClass::setBeat(ofxAubioBeat & _beat) {
    beat = &_beat;
    ofAddListener(beat->gotBeat, this, &ofxAubioAttackClass::beatEvent);
}

void ofxAubioAttackClass::audioIn()
{
    energies.push_back(bands->energies);
    if (energies.size() > max(lag_onset, lag_beat) - 1) {
        energies.erase (energies.begin());
    }
    // hack to add a counter to delay lag * blockSize frames
    if (startOnsetSelection > 0) {
        startOnsetSelection--;
        if (startOnsetSelection == 1) {
            onsetClassify();
        }
    }
    if (startBeatSelection > 0) {
        startBeatSelection--;
        if (startBeatSelection == 1) {
            beatClassify();
        }
    }
}

void ofxAubioAttackClass::onsetEvent(float & time)
{
    //ofLog() << "ofxAubioAttackClass got onset at " << time;
    // hack to add a counter to delay lag * blockSize frames
    int delay = 0;
    startOnsetSelection = lag_onset + delay;
}

void ofxAubioAttackClass::onsetClassify() {
    if (energies.size() == lag_onset - 1) {
        int max_band = 0;
        float max_energy = 0;
        for (int i = 0; i < bands->nBands; i ++) {
            float band_sum = 0;
            for (int j = 0; j < energies.size(); j ++) {
                band_sum += energies[j][i];
            }
            if (max_energy < band_sum) {
                max_energy = band_sum;
                max_band = i;
            }
        }
        currentClass = max_band;
        ofNotifyEvent(gotOnsetClass, currentClass, this);
        ofNotifyEvent(gotGlobalOnsetClass, currentClass);
    }
}

void ofxAubioAttackClass::beatEvent(float & time)
{
    //ofLog() << "ofxAubioAttackClass got onset at " << time;
    // hack to add a counter to delay lag * blockSize frames
    int delay = 0;
    startBeatSelection = lag_beat + delay;
}

void ofxAubioAttackClass::beatClassify() {
    if (energies.size() == lag_beat - 1) {
        int max_band = 0;
        float max_energy = 0;
        for (int i = 0; i < bands->nBands; i ++) {
            float band_sum = 0;
            for (int j = 0; j < energies.size(); j ++) {
                band_sum += energies[j][i];
            }
            if (max_energy < band_sum) {
                max_energy = band_sum;
                max_band = i;
            }
        }
        currentClass = max_band;
        ofNotifyEvent(gotBeatClass, currentClass, this);
        ofNotifyEvent(gotGlobalBeatClass, currentClass);
    }
}