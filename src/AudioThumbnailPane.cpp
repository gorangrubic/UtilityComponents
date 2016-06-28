#include "UtilityComponents/AudioThumbnailPane.hpp"

#include "UtilityComponents/Lerp.hpp"

AudioThumbnailPane::AudioThumbnailPane(
        juce::AudioFormatManager &audio_format_manager,
        TransportViewManager &transport_view_manager)
        : transport_view_manager(transport_view_manager)
        , audio_thumbnail_cache(16)
        , audio_thumbnail(256, audio_format_manager, audio_thumbnail_cache) {
    addAndMakeVisible(playhead);

    transport_view_manager.addListener(this);
    audio_thumbnail.addChangeListener(this);
    playhead.addListener(this);
}

AudioThumbnailPane::~AudioThumbnailPane() noexcept {
    transport_view_manager.removeListener(this);
    audio_thumbnail.removeChangeListener(this);
    playhead.removeListener(this);
}

void AudioThumbnailPane::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);
    auto num_channels = audio_thumbnail.getNumChannels();
    for (auto i = 0; i != num_channels; ++i) {
        auto start = i * getHeight() / num_channels;
        g.setFillType(juce::FillType(
                juce::ColourGradient(juce::Colours::white,
                                     0,
                                     start,
                                     juce::Colours::darkgrey,
                                     0,
                                     start + getHeight() / num_channels,
                                     false)));
        audio_thumbnail.drawChannel(
                g,
                juce::Rectangle<int>(
                        0, start, getWidth(), getHeight() / num_channels),
                transport_view_manager.get_visible_range().getStart(),
                transport_view_manager.get_visible_range().getEnd(),
                i,
                1.0);
    }
}
void AudioThumbnailPane::resized() {
    playhead.setSize(5, getHeight());
    position_playhead();
    repaint();
}

void AudioThumbnailPane::changeListenerCallback(juce::ChangeBroadcaster *cb) {
    position_playhead();
    repaint();
}

void AudioThumbnailPane::set_reader(juce::AudioFormatReader *new_reader, juce::int64 hash) {
    audio_thumbnail.setReader(new_reader, hash);
}

void AudioThumbnailPane::max_range_changed(PlaybackViewManager *r,
                                           const juce::Range<double> &range) {
    position_playhead();
    repaint();
}
void AudioThumbnailPane::visible_range_changed(PlaybackViewManager *r,
                                               const juce::Range<double> &range) {
    position_playhead();
    repaint();
}
void AudioThumbnailPane::current_time_changed(PlaybackViewManager *r,
                                              double time) {
    position_playhead();
    repaint();
}

void AudioThumbnailPane::playhead_dragged(Playhead *p, const juce::MouseEvent &e) {
    if (p == &playhead &&
        !transport_view_manager.audio_transport_source.isPlaying()) {
        auto mouse_pos = e.getEventRelativeTo(this).getPosition().getX();
        auto constrained = juce::Range<int>(1, getWidth() - 1).clipValue(mouse_pos);
        transport_view_manager.audio_transport_source.setPosition(
                x_to_time(constrained));
        transport_view_manager.timerCallback();
    }
}

double AudioThumbnailPane::time_to_x(double t) const {
    return lerp(t,
                transport_view_manager.get_visible_range(),
                juce::Range<double>(0, getWidth()));
}
double AudioThumbnailPane::x_to_time(double t) const {
    return lerp(t,
                juce::Range<double>(0, getWidth()),
                transport_view_manager.get_visible_range());
}
void AudioThumbnailPane::position_playhead() {
    playhead.setTopLeftPosition(
            time_to_x(transport_view_manager.audio_transport_source
                              .getCurrentPosition()) -
                    (playhead.getWidth() * 0.5),
            0);
}
