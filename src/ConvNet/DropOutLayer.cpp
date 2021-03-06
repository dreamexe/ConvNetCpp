﻿#include "Layers.h"


namespace ConvNet {


DropOutLayer::DropOutLayer(double drop_prob) : drop_prob(drop_prob) {
	if (!(drop_prob >= 0.0 && drop_prob <= 1.0))
		throw ArgumentException("DropOutLayer probability is not valid");
	
}

void DropOutLayer::Init(int input_width, int input_height, int input_depth) {
	LayerBase::Init(input_width, input_height, input_depth);
	
	// computed
	output_width = input_width;
	output_height = input_height;
	output_depth = input_depth;
	
	dropped.SetCount(0);
	dropped.SetCount(output_width * output_height * output_depth, false);
}

Volume& DropOutLayer::Forward(Volume& input, bool is_training) {
	input_activation = &input;
	output_activation = input;
	Volume& output = output_activation;
	
	int length = input.GetLength();
	
	if (is_training) {
		// do dropout
		for (int i = 0; i < length; i++) {
			if (Randomf() < drop_prob) {
				output.Set(i, 0);
				dropped[i] = true;
			} // drop!
			else {
				dropped[i] = false;
			}
		}
	}
	else {
		// scale the activations during prediction
		for (int i = 0; i < length; i++) {
			// NOTE:
			//  in direct C# version translation: output->Set(i, output.Get(i) * (1 - drop_prob));
			//  but in original JS version was V2.w[i]*=drop_prob;
			output.Set(i, output.Get(i) * drop_prob);
		}
	}
	
	return output_activation; // dummy identity function for now
}

void DropOutLayer::Backward() {
	Volume& input = *input_activation; // we need to set dw of this
	Volume& output = output_activation;
	
	int length = input.GetLength();
	input.ZeroGradients(); // zero out gradient wrt data
	
	for (int i = 0; i < length; i++) {
		if (!dropped[i]) {
			input.SetGradient(i, output.GetGradient(i)); // copy over the gradient
		}
	}
}

#define STOREVAR(json, field) map.GetAdd(#json) = this->field;
#define LOADVAR(field, json) this->field = map.GetValue(map.Find(#json));
#define LOADVARDEF(field, json, def) {Value tmp = map.GetValue(map.Find(#json)); if (tmp.IsNull()) this->field = def; else this->field = tmp;}

void DropOutLayer::Store(ValueMap& map) const {
	STOREVAR(out_depth, output_depth);
	STOREVAR(out_sx, output_width);
	STOREVAR(out_sy, output_height);
	STOREVAR(layer_type, GetKey());
	STOREVAR(drop_prob, drop_prob);
}

void DropOutLayer::Load(const ValueMap& map) {
	LOADVAR(output_depth, out_depth);
	LOADVAR(output_width, out_sx);
	LOADVAR(output_height, out_sy);
	LOADVAR(drop_prob, drop_prob);
}

String DropOutLayer::ToString() const {
	return Format("Dropout: w:%d, h:%d, d:%d, probability:%2!,n",
		output_width, output_height, output_depth, drop_prob);
}

}
