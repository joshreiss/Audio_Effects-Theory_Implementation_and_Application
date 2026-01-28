/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PanningAudioProcessor::PanningAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
  : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
    .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
  )
#endif
{
  addParameter(methodParam = new juce::AudioParameterChoice("Method", "Method", { "Panorama + Precedence", "ITD + ILD"}, 0));
  addParameter(coneOuterGainParam = new juce::AudioParameterFloat("coneOuterGain", "coneOuterGain", -1.0f, 1.0, 0.0f));
  addParameter(coneInnerAngleParam = new juce::AudioParameterFloat("coneInnerAngle", "coneInnerAngle", -1.0f, 1.0, 0.0f));
  addParameter(coneOuterAngleParam = new juce::AudioParameterFloat("coneOuterAngle", "coneOuterAngle", -1.0f, 1.0, 0.0f));
  addParameter(refDistanceParam = new juce::AudioParameterFloat("refDistance", "refDistance", -1.0f, 1.0, 0.0f));
  addParameter(rolloffParam = new juce::AudioParameterFloat("Rolloff", "Rolloff", -1.0f, 1.0, 0.0f));
  addParameter(panningParam = new juce::AudioParameterFloat("Panning", "Panning", -1.0f, 1.0, 0.0f));

}

PanningAudioProcessor::~PanningAudioProcessor()
{
}

//==============================================================================
const juce::String PanningAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool PanningAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PanningAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PanningAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PanningAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int PanningAudioProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int PanningAudioProcessor::getCurrentProgram()
{
  return 0;
}

void PanningAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PanningAudioProcessor::getProgramName(int index)
{
  return {};
}

void PanningAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void PanningAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  const double smoothTime = 1e-3;
  maximumDelayInSamples = (int)(1e-3f * (float)getSampleRate());
  delayLineL.setup(maximumDelayInSamples);
  delayLineR.setup(maximumDelayInSamples);
}

void PanningAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PanningAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
    && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void PanningAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Helpful information about this block of samples:
  const int numInputChannels = getTotalNumInputChannels();    // How many input channels for our effect?
  const int numOutputChannels = getTotalNumOutputChannels();  // How many output channels for our effect?
  const int numSamples = buffer.getNumSamples();              // How many samples in the buffer for this block?
  float sampleRate = (float)juce::AudioProcessor::getSampleRate();
  auto method = methodParam->getIndex();
  auto panning = panningParam->get();
  // Assume we have these values from the interface
    float sourcePosition[] = { 1, 1 };
  float listenerPosition[] = { 0, 0 };
  // Directivity parameters
  float sourceOrientation[] = { 0, -1 };
  float coneInnerAngle = 0;
  float coneOuterAngle = 180;
  float coneOuterGain = 0.5;
  // Distance parameters
  float rollOff = 1;
  float refDistance = 1;
  float* channelDataL = buffer.getWritePointer(0);
  float* channelDataR = buffer.getWritePointer(1);
  // Panning parameters
  float listenerForward[] = { 0, 1 };

    float* listenerToSource = difference(sourcePosition, listenerPosition);
  /* GAIN FROM SOUND CONE */
  // Find angle between source orientation vector and source-listener vector 
  float* sourceToListener = difference(listenerPosition, sourcePosition);
  float angle = AngleBetweenVectors(sourceToListener, sourceOrientation, 0);
  // Compute gain due to sound cone
  float coneGain;
  if (2 * angle < coneInnerAngle) coneGain = 1.0;
  else if (2 * angle > coneOuterAngle) coneGain = coneOuterGain;
  else coneGain = 1.0 + (coneOuterGain - 1.0) * (2 * angle - coneInnerAngle) / (coneOuterAngle - coneInnerAngle);

  /* GAIN FROM DISTANCE */
  // Find distance between listener and source
  float distance = magnitude(listenerToSource);
  // Compute gain due to distance
  float distanceGain;
  if (distance < refDistance) distanceGain = 1;
  else distanceGain = refDistance / (refDistance + rollOff * (distance - refDistance));

  /* PANNING */
  // Compute azimuth angle
  float azimuth = AngleBetweenVectors(listenerForward, listenerToSource, 1);
  float gainLeft = cos((azimuth / 2.0 + 45) * M_PI / 180.0);
  float gainRight = sin((azimuth / 2.0 + 45) * M_PI / 180.0);

  for (int sample = 0; sample < numSamples; ++sample) {
      channelDataL[sample] = channelDataL[sample] * gainLeft * distanceGain * coneGain;
      channelDataR[sample] = channelDataR[sample] * gainRight * distanceGain * coneGain;
  }

  // In case we have more outputs than inputs, clear any output channels that didn't contain input data
  for (int i = numInputChannels; i < numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool PanningAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PanningAudioProcessor::createEditor()
{
  return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void PanningAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void PanningAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new PanningAudioProcessor();
}
