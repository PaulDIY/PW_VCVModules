#include "plugin.hpp"


struct No02 : Module {
    
    const int numOSCs = 5;
    
    enum ParamId {
        ENUMS(OSC_PITCH, 5),
        ENUMS(OSC_LVL, 5),
        ENUMS(OSC_SWITCH, 5),
        OUTPUT_LVL,
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
        AUDIO_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
        ENUMS(PHASE_LIGHTS, 5),
        ENUMS(MUTE_LIGHTS, 5),
		LIGHTS_LEN
	};

	No02() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN); // string::f("Row %d mute", i + 1)
        
        for (int i = 0; i < numOSCs; i++) {
            configParam(OSC_PITCH+i, -12.0f, 12.f, 0.f, string::f("OSC %d Tune", i+1));
            configSwitch(OSC_SWITCH+i, 0.f, 1.f, 0.f, string::f("OSC %d activate", i+1));
            configParam(OSC_LVL+i, 0.0f, 2.f, 1.0f, string::f("OSC %d Volume", i+1), "db", -10, 40);
        };
        configParam(OUTPUT_LVL, 0.0f, 2.f, 1.0f, "Output Volume", "db", -10, 40);
		configInput(PITCH_INPUT, "1 V/oct");
		configOutput(AUDIO_OUTPUT, "Output");
	}

	float phase = 0.f;
    float phases[5] = {0.0f, 0.1f, 0.3f, 0.2f, 0.4f};    // should be random(-0.5, 0.5)
    float offsets[5] = {-36.0f/12.0f, -24.1f/12.0f, -11.95f/12.0f, 0.0f, 12.2f/12.0f}; // pitch offset of OSCs
    
    
	void process(const ProcessArgs& args) override {
		
        float sum = 0.f;
        // Compute the frequency from the pitch parameter and input
        
        float inputVolts = inputs[PITCH_INPUT].getVoltage();
        
        // add some bad power supply (pitch drops with numActive)
        int numActive = 0;
        for (int i = 0; i < numOSCs; i++) {
            numActive += (params[OSC_SWITCH + i].getValue() > 0.f);
        };
        
        // cycle through all OSCs
        for (int i = 0; i < numOSCs; i++) {
            // read ON/OFF switch
            bool active = params[OSC_SWITCH + i].getValue() > 0.f;
            // read pitch parameter
            float pitch = params[OSC_PITCH+i].getValue()/12.0f;
            // read volume parameter
            float volume = params[OSC_LVL+i].getValue() / (i+1.0f);
            // calc freq from cv-input and pitch parameter
            float freq = dsp::FREQ_C4 * std::pow(2.f, pitch + offsets[i] + inputVolts - (numActive * 0.005));
            // calc phase
            phases[i] += freq * args.sampleTime;
            if (phases[i] >= 0.5f) phases[i] -= 1.f;
            // multiply by volume and add to sum
            
            float saw = 0;
            for (int k = 1; k<10; k++) {
                saw += std::pow(-1, k) * (std::sin(2.f * M_PI * k * phases[i]) / k );
            }
            saw *= 2.00f/M_PI;
            
            float sine = std::sin(2.f * M_PI * phases[i]);
            
            // OSC 1 == sine
            if (i == 0) { saw = sine; }
            
            sum += active * volume * (saw);
            
            // Set light
            lights[PHASE_LIGHTS + i].setBrightness(phases[i]);
            lights[MUTE_LIGHTS + i].setBrightness(active);
        };
        
		// Audio signals are typically +/-5V
		// https://vcvrack.com/manual/VoltageStandards
        float volume = params[OUTPUT_LVL].getValue();
        outputs[AUDIO_OUTPUT].setVoltage(volume * sum);
	}

};


struct No02Widget : ModuleWidget {
	No02Widget(No02* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/No02.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        int numOSCs = 5;//No02::numOSCs;
        float x = 20;
        float dx = 18;
        float y = 79.58;
        float dy = -15.54;
        
        for (int i = 0; i < numOSCs; i++) {
            addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x, y)), module, No02::OSC_PITCH + i));
            addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(x+dx, y)), module, No02::PHASE_LIGHTS + i));
            addParam(createLightParamCentered<VCVLightBezelLatch<>>(mm2px(Vec(x+2*dx, y)), module, No02::OSC_SWITCH + i, No02::MUTE_LIGHTS + i));
            //addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x+2*dx, y)), module, No02::OSC_SWITCH + i));
            addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x+3*dx, y)), module, No02::OSC_LVL + i));
            y += dy;
        };
        
        //addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x+3*dx, y+ 0.5f*dy)), module, No02::OUTPUT_LVL));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x+3*dx, 103)), module, No02::OUTPUT_LVL));
        
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 113.74)), module, No02::PITCH_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(71.12, 113.74)), module, No02::AUDIO_OUTPUT));
	}

};


Model* modelNo02 = createModel<No02, No02Widget>("No02");
