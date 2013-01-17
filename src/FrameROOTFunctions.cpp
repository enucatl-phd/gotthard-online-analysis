#include "FrameROOTFunctions.h"

namespace frame_root_functions {

int get_histogram(const FullFrame& frame, TH1D& histogram) {
    if (frame.size() != histogram.GetNbinsX()) return -1;
    for (int i = 0; i < frame.size(); i++) {
        histogram.SetBinContent(i + 1, frame.get_pixel(i));
    }
    return 0;
}

int get_average(const std::vector<FullFrame>& frames, TH1D& histogram){
    //check that the vector is not empty
    int number_of_frames = frames.size();
    if (!number_of_frames) return -1;

    //check that the histogram has the right number of bins
    int number_of_pixels = frames[0].size();
    if (number_of_pixels != histogram.GetNbinsX()) return -1;

    for (int i = 0; i < number_of_pixels; i++) {
        std::vector<unsigned int> values;
        values.reserve(number_of_frames);
        for (std::vector<FullFrame>::const_iterator frame = frames.begin(); frame != frames.end(); ++frame) {
            values.push_back(frame->get_pixel(i));
        }
        double mean = TMath::Mean(values.begin(), values.end());
        //error on the mean: sigma / sqrt(n)
        double error = TMath::RMS(values.begin(), values.end()) / TMath::Sqrt(number_of_frames);
        histogram.SetBinContent(i + 1, mean);
        histogram.SetBinError(i + 1, error);
    }
    return 0;
}

int get_histogram_subtracting_pedestal(const FullFrame& frame, TH1D& pedestal, TH1D& histogram) {
    int result = get_histogram(frame, histogram);
    if (!result)
        histogram.Sumw2();
        histogram.Add(&pedestal, -1);
    return result;
}

int get_histograms_subtracting_pedestal(const std::vector<FullFrame>& frames, TH1D& pedestal, std::vector<TH1D>& histograms) {
    //histograms should be empty
    histograms.clear();
    histograms.reserve(frames.size());
    for (std::vector<FullFrame>::const_iterator frame = frames.begin(); frame != frames.end(); ++frame) {
        std::stringstream histogram_name;
        histogram_name << "frame" << frame->frame1_number() << frame->frame2_number();
        TH1D histogram(
                histogram_name.str().c_str(),
                histogram_name.str().c_str(),
                frame->size(),
                0,
                frame->size());
        int result = get_histogram_subtracting_pedestal(*frame, pedestal, histogram);
        if (result != 0) return result;
        histograms.push_back(histogram);
    }
    return 0;
}

void get_photon_tree(const std::vector<TH1D>& histograms, TTree& tree){
    double energy;
    tree.Branch("energy", &energy, "energy/D");
    for (std::vector<TH1D>::const_iterator histogram = histograms.begin(); histogram != histograms.end(); ++histogram) {
        for (int i = 0; i < histogram->GetNbinsX(); i++) {
            energy = histogram->GetBinContent(i + 1);
            tree.Fill();
        }
    }
}
}