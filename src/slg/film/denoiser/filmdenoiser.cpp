/***************************************************************************
 * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxCoreRender.                                   *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <limits>
#include <algorithm>
#include <exception>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "slg/film/film.h"
#include "slg/film/sampleresult.h"
#include "slg/film/denoiser/filmdenoiser.h"
#include "slg/film/imagepipeline/imagepipeline.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// Film BCD denoiser
//------------------------------------------------------------------------------

BOOST_CLASS_EXPORT_IMPLEMENT(slg::FilmDenoiser)

FilmDenoiser::FilmDenoiser() {
	Init();
}

FilmDenoiser::FilmDenoiser(const Film *f) : film(f) {
	Init();

	film = f;
	referenceFilmWidth = film->GetWidth();
	referenceFilmHeight = film->GetHeight();
}

void FilmDenoiser::Init() {
	film = NULL;
	samplesAccumulatorPixelNormalized = NULL;
	samplesAccumulatorScreenNormalized = NULL;
	sampleScale = 1.f;
	warmUpDone = false;

	referenceFilm = NULL;
	referenceFilmWidth = 0;
	referenceFilmHeight = 0;
	referenceFilmOffsetX = 0;
	referenceFilmOffsetY = 0;
	
	filmAddEnabled = true;

	enabled = false;
}

void FilmDenoiser::Clear() {
	if (enabled && warmUpDone && !referenceFilm) {
		if (samplesAccumulatorPixelNormalized)
			samplesAccumulatorPixelNormalized->Clear();
		if (samplesAccumulatorScreenNormalized)
			samplesAccumulatorScreenNormalized->Clear();
	}
}

FilmDenoiser::~FilmDenoiser() {
	if (!referenceFilm) {
		delete samplesAccumulatorPixelNormalized;
		delete samplesAccumulatorScreenNormalized;
	}
}

float *FilmDenoiser::GetNbOfSamplesImage() {
	if (samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->m_samplesStatisticsImages.m_nbOfSamplesImage.getDataPtr();
	else
		return NULL;
}

float *FilmDenoiser::GetSquaredWeightSumsImage() {
	if (samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->m_squaredWeightSumsImage.getDataPtr();
	else
		return NULL;
}

float *FilmDenoiser::GetMeanImage() {
	if (samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->m_samplesStatisticsImages.m_meanImage.getDataPtr();
	else
		return NULL;
}

float *FilmDenoiser::GetCovarImage() {
	if (samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->m_samplesStatisticsImages.m_covarImage.getDataPtr();
	else
		return NULL;
}

float *FilmDenoiser::GetHistoImage() {
	if (samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->m_samplesStatisticsImages.m_histoImage.getDataPtr();
	else
		return NULL;
}

void FilmDenoiser::CheckReferenceFilm() {
	if (referenceFilm->filmDenoiser.warmUpDone) {
		sampleScale = referenceFilm->filmDenoiser.sampleScale;
		radianceChannelScales = referenceFilm->filmDenoiser.radianceChannelScales;
		samplesAccumulatorPixelNormalized = referenceFilm->filmDenoiser.samplesAccumulatorPixelNormalized;
		samplesAccumulatorScreenNormalized = referenceFilm->filmDenoiser.samplesAccumulatorScreenNormalized;

		warmUpDone = true;
	}
}

void FilmDenoiser::SetReferenceFilm(const Film *refFilm,
		const u_int offsetX, const u_int offsetY,
		const bool addEnabled) {
	referenceFilm = refFilm;
	filmAddEnabled = addEnabled;
	
	if (referenceFilm) {
		referenceFilmWidth = referenceFilm->GetWidth();
		referenceFilmHeight = referenceFilm->GetHeight();
		referenceFilmOffsetX = offsetX;
		referenceFilmOffsetY = offsetY;

		CheckReferenceFilm();
	}
}

void FilmDenoiser::Reset() {
	if (!referenceFilm) {
		delete samplesAccumulatorPixelNormalized;
		delete samplesAccumulatorScreenNormalized;
	} else {
		// In case of a reference film resize
		referenceFilmWidth = referenceFilm->GetWidth();
		referenceFilmHeight = referenceFilm->GetHeight();
	}

	samplesAccumulatorPixelNormalized = NULL;
	samplesAccumulatorScreenNormalized = NULL;
	radianceChannelScales.clear();
	sampleScale = 1.f;
	warmUpDone = false;
}

void FilmDenoiser::WarmUpDone() {
	SLG_LOG("BCD denoiser warmup done");

	// Get the current film luminance
	// I use the pipeline of the first BCD plugin
	const u_int denoiserImagePipelineIndex = ImagePipelinePlugin::GetBCDPipelineIndex(*film);
	radianceChannelScales = film->GetImagePipeline(denoiserImagePipelineIndex)->radianceChannelScales;

	// Adjust the ray fusion histogram as if I'm using auto-linear tone mapping
	const float filmY = film->GetFilmY(denoiserImagePipelineIndex);
	sampleScale = (filmY == 0.f) ? 1.f : (1.25f / filmY * powf(118.f / 255.f, 2.2f));

	// Allocate denoiser samples collectors
	if (film->HasChannel(Film::RADIANCE_PER_PIXEL_NORMALIZED))
		samplesAccumulatorPixelNormalized = new SamplesAccumulator(film->GetWidth(), film->GetHeight(),
				bcd::HistogramParameters());
	if (film->HasChannel(Film::RADIANCE_PER_SCREEN_NORMALIZED))
		samplesAccumulatorScreenNormalized = new SamplesAccumulator(film->GetWidth(), film->GetHeight(),
				bcd::HistogramParameters());

	// This will trigger the thread using this film as reference
	warmUpDone = true;
}

bool FilmDenoiser::HasSamplesStatistics(const bool pixelNormalizedSampleAccumulator) const {
	if (pixelNormalizedSampleAccumulator && samplesAccumulatorPixelNormalized)
		return true;
	else if (!pixelNormalizedSampleAccumulator && samplesAccumulatorScreenNormalized)
		return true;
	else
		return false;
}

bcd::SamplesStatisticsImages FilmDenoiser::GetSamplesStatistics(const bool pixelNormalizedSampleAccumulator) const {
if (pixelNormalizedSampleAccumulator && samplesAccumulatorPixelNormalized)
		return samplesAccumulatorPixelNormalized->GetSamplesStatistics();
	else if (!pixelNormalizedSampleAccumulator && samplesAccumulatorScreenNormalized)
		return samplesAccumulatorScreenNormalized->GetSamplesStatistics();
	else
		return bcd::SamplesStatisticsImages();
}

void FilmDenoiser::AddDenoiser(const FilmDenoiser &filmDenoiser,
		const u_int srcOffsetX, const u_int srcOffsetY,
		const u_int srcWidth, const u_int srcHeight,
		const u_int dstOffsetX, const u_int dstOffsetY) {
	if (enabled && samplesAccumulatorPixelNormalized &&
			filmDenoiser.enabled && filmDenoiser.samplesAccumulatorPixelNormalized &&
			!filmDenoiser.referenceFilm) {
		if (samplesAccumulatorPixelNormalized)
			samplesAccumulatorPixelNormalized->AddAccumulator(*filmDenoiser.samplesAccumulatorPixelNormalized,
					(int)srcOffsetX, (int)srcOffsetY,
					(int)srcWidth, (int)srcHeight,
					(int)dstOffsetX, (int)dstOffsetY);
		if (samplesAccumulatorScreenNormalized)
			samplesAccumulatorScreenNormalized->AddAccumulator(*filmDenoiser.samplesAccumulatorScreenNormalized,
					(int)srcOffsetX, (int)srcOffsetY,
					(int)srcWidth, (int)srcHeight,
					(int)dstOffsetX, (int)dstOffsetY);
	}
}

void FilmDenoiser::AddDenoiser(const FilmDenoiser &filmDenoiser) {
	AddDenoiser(filmDenoiser, 0, 0, film->GetWidth(), film->GetHeight(), 0, 0);
}

void FilmDenoiser::AddSample(const u_int x, const u_int y,
		const SampleResult &sampleResult, const float weight) {
	if (!enabled)
		return;

	SamplesAccumulator *samplesAccumulator;
	if (sampleResult.GetChannels() & Film::RADIANCE_PER_PIXEL_NORMALIZED)
		samplesAccumulator = samplesAccumulatorPixelNormalized;
	else if (sampleResult.GetChannels() & Film::RADIANCE_PER_SCREEN_NORMALIZED)
		samplesAccumulator = samplesAccumulatorScreenNormalized;
	else
		samplesAccumulator = NULL;
	
	if (samplesAccumulator) {
		const Spectrum sample = (sampleResult.GetSpectrum(radianceChannelScales) * sampleScale).Clamp(
				0.f, samplesAccumulator->GetHistogramParameters().m_maxValue);

		if (!sample.IsNaN() && !sample.IsInf()) {
			const int line = referenceFilmHeight - (y + referenceFilmOffsetY) - 1;
			const int column = x + referenceFilmOffsetX;

			samplesAccumulator->AddSampleAtomic(line, column,
					sample.c[0], sample.c[1], sample.c[2],
					weight);
		}
	} else {
		// Check if I have to allocate denoiser statistics collector

		if (referenceFilm)
			CheckReferenceFilm();
		else if (film->GetTotalSampleCount() / (film->GetWidth() * film->GetHeight()) > 2.0) {
			// The warmup period is over and I can allocate denoiser SamplesAccumulator

			WarmUpDone();
		}
	}
}
