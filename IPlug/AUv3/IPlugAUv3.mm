#import <AVFoundation/AVFoundation.h>
#include "BufferedAudioBus.hpp"

#import "IPlugAUv3.h"
#include "IPlugDSPKernel.hpp"

@interface IPlugAUv3 ()

@property AUAudioUnitBus *outputBus;
@property AUAudioUnitBusArray *inputBusArray;
@property AUAudioUnitBusArray *outputBusArray;

@end

@implementation IPlugAUv3 {
  IPlugDSPKernel  _kernel;
  BufferedInputBus _inputBus;
  
//  AUAudioUnitPreset   *_currentPreset;
//  NSInteger           _currentFactoryPresetIndex;
//  NSArray<AUAudioUnitPreset *> *_presets;
}
@synthesize parameterTree = _parameterTree;
//@synthesize factoryPresets = _presets;

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription
                                     options:(AudioComponentInstantiationOptions)options
                                       error:(NSError **)outError {
  
  self = [super initWithComponentDescription:componentDescription
                                     options:options
                                       error:outError];
  
  if (self == nil) { return nil; }
  
  // Initialize a default format for the busses.
  AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100.0 channels:2];
  
  // Create a DSP kernel to handle the signal processing.
  _kernel.init(defaultFormat.channelCount, defaultFormat.sampleRate);
  
  // Create a parameter object for the cutoff frequency.
  AUParameter *cutoffParam = [AUParameterTree createParameterWithIdentifier:@"cutoff" name:@"Cutoff"
                                                                    address:0
                                                                        min:12.0 max:20000.0 unit:kAudioUnitParameterUnit_Hertz unitName:nil
                                                                      flags: kAudioUnitParameterFlag_IsReadable |
                              kAudioUnitParameterFlag_IsWritable |
                              kAudioUnitParameterFlag_CanRamp
                                                               valueStrings:nil dependentParameters:nil];

  // Create a parameter object for the filter resonance.
  AUParameter *resonanceParam = [AUParameterTree createParameterWithIdentifier:@"resonance" name:@"Resonance"
                                                                       address:1
                                                                           min:-20.0 max:20.0 unit:kAudioUnitParameterUnit_Decibels unitName:nil
                                                                         flags: kAudioUnitParameterFlag_IsReadable |
                                 kAudioUnitParameterFlag_IsWritable |
                                 kAudioUnitParameterFlag_CanRamp
                                                                  valueStrings:nil dependentParameters:nil];
  
// Initialize default parameter values.
  cutoffParam.value = 20000.0;
  resonanceParam.value = 0.0;
  _kernel.setParameter(0, cutoffParam.value);
  _kernel.setParameter(1, resonanceParam.value);
  
  // Create factory preset array.
//  _currentFactoryPresetIndex = kDefaultFactoryPreset;
//  _presets = @[NewAUPreset(0, @"First Preset"),
//               NewAUPreset(1, @"Second Preset"),
//               NewAUPreset(2, @"Third Preset")];
  
  // Create the parameter tree.
  _parameterTree = [AUParameterTree createTreeWithChildren:@[cutoffParam, resonanceParam]];
  _parameterTree = [AUParameterTree alloc];

  // Create the input and output busses.
  _inputBus.init(defaultFormat, 8);
  _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
  
  // Create the input and output bus arrays.
  _inputBusArray  = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeInput busses: @[_inputBus.bus]];
  _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses: @[_outputBus]];
  
  // Make a local pointer to the kernel to avoid capturing self.
  __block IPlugDSPKernel *dspKernel = &_kernel;

  // implementorValueObserver is called when a parameter changes value.
  _parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
    dspKernel->setParameter(param.address, value);
  };

  // implementorValueProvider is called when the value needs to be refreshed.
  _parameterTree.implementorValueProvider = ^(AUParameter *param) {
    return dspKernel->getParameter(param.address);
  };
  
  // A function to provide string representations of parameter values.
  _parameterTree.implementorStringFromValueCallback = ^(AUParameter *param, const AUValue *__nullable valuePtr) {
    AUValue value = valuePtr == nil ? param.value : *valuePtr;
    
    switch (param.address) {
      case 0:
        return [NSString stringWithFormat:@"%.f", value];

      case 1:
        return [NSString stringWithFormat:@"%.2f", value];

      default:
        return @"?";
    }
  };
  
  self.maximumFramesToRender = 512;
  
//  // set default preset as current
//  self.currentPreset = _presets.firstObject;
  
  return self;
}

-(void)dealloc {
//  _presets = nil;
}

#pragma mark - AUAudioUnit (Overrides)

- (AUAudioUnitBusArray *)inputBusses {
  return _inputBusArray;
}

- (AUAudioUnitBusArray *)outputBusses {
  return _outputBusArray;
}

- (BOOL)allocateRenderResourcesAndReturnError:(NSError **)outError {
  if (![super allocateRenderResourcesAndReturnError:outError]) {
    return NO;
  }
  
  if (self.outputBus.format.channelCount != _inputBus.bus.format.channelCount) {
    if (outError) {
      *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FailedInitialization userInfo:nil];
    }
    // Notify superclass that initialization was not successful
    self.renderResourcesAllocated = NO;
    
    return NO;
  }
  
  _inputBus.allocateRenderResources(self.maximumFramesToRender);
  
  _kernel.init(self.outputBus.format.channelCount, self.outputBus.format.sampleRate);
  _kernel.reset();
  
  return YES;
}

- (void)deallocateRenderResources {
  _inputBus.deallocateRenderResources();
  
  [super deallocateRenderResources];
}

#pragma mark - AUAudioUnit (AUAudioUnitImplementation)

- (AUInternalRenderBlock)internalRenderBlock {
  /*
   Capture in locals to avoid ObjC member lookups. If "self" is captured in
   render, we're doing it wrong.
   */
  // Specify captured objects are mutable.
  __block IPlugDSPKernel *state = &_kernel;
  __block BufferedInputBus *input = &_inputBus;

  return ^AUAudioUnitStatus(
                            AudioUnitRenderActionFlags *actionFlags,
                            const AudioTimeStamp       *timestamp,
                            AVAudioFrameCount           frameCount,
                            NSInteger                   outputBusNumber,
                            AudioBufferList            *outputData,
                            const AURenderEvent        *realtimeEventListHead,
                            AURenderPullInputBlock      pullInputBlock) {
    AudioUnitRenderActionFlags pullFlags = 0;

    AUAudioUnitStatus err = input->pullInput(&pullFlags, timestamp, frameCount, 0, pullInputBlock);

    if (err != 0) { return err; }
    
    AudioBufferList *inAudioBufferList = input->mutableAudioBufferList;

    /*
     Important:
     If the caller passed non-null output pointers (outputData->mBuffers[x].mData), use those.

     If the caller passed null output buffer pointers, process in memory owned by the Audio Unit
     and modify the (outputData->mBuffers[x].mData) pointers to point to this owned memory.
     The Audio Unit is responsible for preserving the validity of this memory until the next call to render,
     or deallocateRenderResources is called.

     If your algorithm cannot process in-place, you will need to preallocate an output buffer
     and use it here.

     See the description of the canProcessInPlace property.
     */

    // If passed null output buffer pointers, process in-place in the input buffer.
    AudioBufferList *outAudioBufferList = outputData;
    if (outAudioBufferList->mBuffers[0].mData == nullptr) {
      for (UInt32 i = 0; i < outAudioBufferList->mNumberBuffers; ++i) {
        outAudioBufferList->mBuffers[i].mData = inAudioBufferList->mBuffers[i].mData;
      }
    }

    state->setBuffers(inAudioBufferList, outAudioBufferList);
    state->processWithEvents(timestamp, frameCount, realtimeEventListHead);

    return noErr;
  };
}

#pragma mark- AUAudioUnit (Optional Properties)

//- (AUAudioUnitPreset *)currentPreset
//{
//  if (_currentPreset.number >= 0) {
//    NSLog(@"Returning Current Factory Preset: %ld\n", (long)_currentFactoryPresetIndex);
//    return [_presets objectAtIndex:_currentFactoryPresetIndex];
//  } else {
//    NSLog(@"Returning Current Custom Preset: %ld, %@\n", (long)_currentPreset.number, _currentPreset.name);
//    return _currentPreset;
//  }
//}

//- (void)setCurrentPreset:(AUAudioUnitPreset *)currentPreset
//{
//  if (nil == currentPreset) { NSLog(@"nil passed to setCurrentPreset!"); return; }
//
//  if (currentPreset.number >= 0) {
//    // factory preset
//    for (AUAudioUnitPreset *factoryPreset in _presets) {
//      if (currentPreset.number == factoryPreset.number) {
//
//        AUParameter *cutoffParameter = [self.parameterTree valueForKey: @"cutoff"];
//        AUParameter *resonanceParameter = [self.parameterTree valueForKey: @"resonance"];
//
//        cutoffParameter.value = presetParameters[factoryPreset.number].cutoffValue;
//        resonanceParameter.value = presetParameters[factoryPreset.number].resonanceValue;
//
//        // set factory preset as current
//        _currentPreset = currentPreset;
//        _currentFactoryPresetIndex = factoryPreset.number;
//        NSLog(@"currentPreset Factory: %ld, %@\n", (long)_currentFactoryPresetIndex, factoryPreset.name);
//
//        break;
//      }
//    }
//  } else if (nil != currentPreset.name) {
//    // set custom preset as current
//    _currentPreset = currentPreset;
//    NSLog(@"currentPreset Custom: %ld, %@\n", (long)_currentPreset.number, _currentPreset.name);
//  } else {
//    NSLog(@"setCurrentPreset not set! - invalid AUAudioUnitPreset\n");
//  }
//}

- (BOOL)canProcessInPlace {
  return NO;
}

@end