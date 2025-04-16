//
// library is free to use with condition that copyright is not removed
// special created by tg @zeronex1337
//
#pragma once
#include <chrono>
#include <functional>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <optional>

#define _USE_MATH_DEFINES
#include <math.h>

#if __has_include(<imgui/imgui.h>)
#include <imgui/imgui.h>
#endif

namespace ImAnim {

    enum class Easing {
        LinearInterpolation,
        QuadraticEaseIn,
        QuadraticEaseOut,
        QuadraticEaseInOut,
        CubicEaseIn,
        CubicEaseOut,
        CubicEaseInOut,
        QuarticEaseIn,
        QuarticEaseOut,
        QuarticEaseInOut,
        QuinticEaseIn,
        QuinticEaseOut,
        QuinticEaseInOut,
        SineEaseIn,
        SineEaseOut,
        SineEaseInOut,
        CircularEaseIn,
        CircularEaseOut,
        CircularEaseInOut,
        ExponentialEaseIn,
        ExponentialEaseOut,
        ExponentialEaseInOut,
        ElasticEaseIn,
        ElasticEaseOut,
        ElasticEaseInOut,
        BackEaseIn,
        BackEaseOut,
        BackEaseInOut,
        BounceEaseIn,
        BounceEaseOut,
        BounceEaseInOut
    };

    // Easing functions taken from 
    // https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
    // Within unlicense

    namespace EasingFunctions {
        // Modeled after the line y = x
        inline float LinearInterpolation(float p)
        {
            return p;
        }

        // Modeled after the parabola y = x^2
        inline float QuadraticEaseIn(float p)
        {
            return p * p;
        }

        // Modeled after the parabola y = -x^2 + 2x
        inline float QuadraticEaseOut(float p)
        {
            return -(p * (p - 2));
        }

        // Modeled after the piecewise quadratic
        // y = (1/2)((2x)^2)             ; [0, 0.5)
        // y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
        inline float QuadraticEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 2 * p * p;
            }
            else
            {
                return (-2 * p * p) + (4 * p) - 1;
            }
        }

        // Modeled after the cubic y = x^3
        inline float CubicEaseIn(float p)
        {
            return p * p * p;
        }

        // Modeled after the cubic y = (x - 1)^3 + 1
        inline float CubicEaseOut(float p)
        {
            float f = (p - 1);
            return f * f * f + 1;
        }

        // Modeled after the piecewise cubic
        // y = (1/2)((2x)^3)       ; [0, 0.5)
        // y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
        inline float CubicEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 4 * p * p * p;
            }
            else
            {
                float f = ((2 * p) - 2);
                return 0.5 * f * f * f + 1;
            }
        }

        // Modeled after the quartic x^4
        inline float QuarticEaseIn(float p)
        {
            return p * p * p * p;
        }

        // Modeled after the quartic y = 1 - (x - 1)^4
        inline float QuarticEaseOut(float p)
        {
            float f = (p - 1);
            return f * f * f * (1 - p) + 1;
        }

        // Modeled after the piecewise quartic
        // y = (1/2)((2x)^4)        ; [0, 0.5)
        // y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
        inline float QuarticEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 8 * p * p * p * p;
            }
            else
            {
                float f = (p - 1);
                return -8 * f * f * f * f + 1;
            }
        }

        // Modeled after the quintic y = x^5
        inline float QuinticEaseIn(float p)
        {
            return p * p * p * p * p;
        }

        // Modeled after the quintic y = (x - 1)^5 + 1
        inline float QuinticEaseOut(float p)
        {
            float f = (p - 1);
            return f * f * f * f * f + 1;
        }

        // Modeled after the piecewise quintic
        // y = (1/2)((2x)^5)       ; [0, 0.5)
        // y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
        inline float QuinticEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 16 * p * p * p * p * p;
            }
            else
            {
                float f = ((2 * p) - 2);
                return  0.5 * f * f * f * f * f + 1;
            }
        }

        // Modeled after quarter-cycle of sine wave
        inline float SineEaseIn(float p)
        {
            return sin((p - 1) * M_PI_2) + 1;
        }

        // Modeled after quarter-cycle of sine wave (different phase)
        inline float SineEaseOut(float p)
        {
            return sin(p * M_PI_2);
        }

        // Modeled after half sine wave
        inline float SineEaseInOut(float p)
        {
            return 0.5 * (1 - cos(p * M_PI));
        }

        // Modeled after shifted quadrant IV of unit circle
        inline float CircularEaseIn(float p)
        {
            return 1 - sqrt(1 - (p * p));
        }

        // Modeled after shifted quadrant II of unit circle
        inline float CircularEaseOut(float p)
        {
            return sqrt((2 - p) * p);
        }

        // Modeled after the piecewise circular function
        // y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
        // y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
        inline float CircularEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
            }
            else
            {
                return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
            }
        }

        // Modeled after the exponential function y = 2^(10(x - 1))
        inline float ExponentialEaseIn(float p)
        {
            return (p == 0.0) ? p : pow(2, 10 * (p - 1));
        }

        // Modeled after the exponential function y = -2^(-10x) + 1
        inline float ExponentialEaseOut(float p)
        {
            return (p == 1.0) ? p : 1 - pow(2, -10 * p);
        }

        // Modeled after the piecewise exponential
        // y = (1/2)2^(10(2x - 1))         ; [0,0.5)
        // y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
        inline float ExponentialEaseInOut(float p)
        {
            if (p == 0.0 || p == 1.0) return p;

            if (p < 0.5)
            {
                return 0.5 * pow(2, (20 * p) - 10);
            }
            else
            {
                return -0.5 * pow(2, (-20 * p) + 10) + 1;
            }
        }

        // Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
        inline float ElasticEaseIn(float p)
        {
            return sin(13 * M_PI_2 * p) * pow(2, 10 * (p - 1));
        }

        // Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
        inline float ElasticEaseOut(float p)
        {
            return sin(-13 * M_PI_2 * (p + 1)) * pow(2, -10 * p) + 1;
        }

        // Modeled after the piecewise exponentially-damped sine wave:
        // y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
        // y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
        inline float ElasticEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 0.5 * sin(13 * M_PI_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
            }
            else
            {
                return 0.5 * (sin(-13 * M_PI_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
            }
        }

        // Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
        inline float BackEaseIn(float p)
        {
            return p * p * p - p * sin(p * M_PI);
        }

        // Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
        inline float BackEaseOut(float p)
        {
            float f = (1 - p);
            return 1 - (f * f * f - f * sin(f * M_PI));
        }

        // Modeled after the piecewise overshooting cubic function:
        // y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
        // y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
        inline float BackEaseInOut(float p)
        {
            if (p < 0.5)
            {
                float f = 2 * p;
                return 0.5 * (f * f * f - f * sin(f * M_PI));
            }
            else
            {
                float f = (1 - (2 * p - 1));
                return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
            }
        }


        inline float BounceEaseOut(float p)
        {
            if (p < 4 / 11.0)
            {
                return (121 * p * p) / 16.0;
            }
            else if (p < 8 / 11.0)
            {
                return (363 / 40.0 * p * p) - (99 / 10.0 * p) + 17 / 5.0;
            }
            else if (p < 9 / 10.0)
            {
                return (4356 / 361.0 * p * p) - (35442 / 1805.0 * p) + 16061 / 1805.0;
            }
            else
            {
                return (54 / 5.0 * p * p) - (513 / 25.0 * p) + 268 / 25.0;
            }
        }

        inline float BounceEaseIn(float p)
        {
            return 1 - BounceEaseOut(1 - p);
        }

        inline float BounceEaseInOut(float p)
        {
            if (p < 0.5)
            {
                return 0.5 * BounceEaseIn(p * 2);
            }
            else
            {
                return 0.5 * BounceEaseOut(p * 2 - 1) + 0.5;
            }
        }
    }

    inline std::function<float(float)> get_easing_func(Easing easing) {
        switch (easing) {
        case Easing::LinearInterpolation:
            return EasingFunctions::LinearInterpolation;
        case Easing::QuadraticEaseIn:
            return EasingFunctions::QuadraticEaseIn;
        case Easing::QuadraticEaseOut:
            return EasingFunctions::QuadraticEaseOut;
        case Easing::QuadraticEaseInOut:
            return EasingFunctions::QuadraticEaseInOut;
        case Easing::CubicEaseIn:
            return EasingFunctions::CubicEaseIn;
        case Easing::CubicEaseOut:
            return EasingFunctions::CubicEaseOut;
        case Easing::CubicEaseInOut:
            return EasingFunctions::CubicEaseInOut;
        case Easing::QuarticEaseIn:
            return EasingFunctions::QuarticEaseIn;
        case Easing::QuarticEaseOut:
            return EasingFunctions::QuarticEaseOut;
        case Easing::QuarticEaseInOut:
            return EasingFunctions::QuarticEaseInOut;
        case Easing::QuinticEaseIn:
            return EasingFunctions::QuinticEaseIn;
        case Easing::QuinticEaseOut:
            return EasingFunctions::QuinticEaseOut;
        case Easing::QuinticEaseInOut:
            return EasingFunctions::QuinticEaseInOut;
        case Easing::SineEaseIn:
            return EasingFunctions::SineEaseIn;
        case Easing::SineEaseOut:
            return EasingFunctions::SineEaseOut;
        case Easing::SineEaseInOut:
            return EasingFunctions::SineEaseInOut;
        case Easing::CircularEaseIn:
            return EasingFunctions::CircularEaseIn;
        case Easing::CircularEaseOut:
            return EasingFunctions::CircularEaseOut;
        case Easing::CircularEaseInOut:
            return EasingFunctions::CircularEaseInOut;
        case Easing::ExponentialEaseIn:
            return EasingFunctions::ExponentialEaseIn;
        case Easing::ExponentialEaseOut:
            return EasingFunctions::ExponentialEaseOut;
        case Easing::ExponentialEaseInOut:
            return EasingFunctions::ExponentialEaseInOut;
        case Easing::ElasticEaseIn:
            return EasingFunctions::ElasticEaseIn;
        case Easing::ElasticEaseOut:
            return EasingFunctions::ElasticEaseOut;
        case Easing::ElasticEaseInOut:
            return EasingFunctions::ElasticEaseInOut;
        case Easing::BackEaseIn:
            return EasingFunctions::BackEaseIn;
        case Easing::BackEaseOut:
            return EasingFunctions::BackEaseOut;
        case Easing::BackEaseInOut:
            return EasingFunctions::BackEaseInOut;
        case Easing::BounceEaseIn:
            return EasingFunctions::BounceEaseIn;
        case Easing::BounceEaseOut:
            return EasingFunctions::BounceEaseOut;
        case Easing::BounceEaseInOut:
            return EasingFunctions::BounceEaseInOut;
        default:
            return EasingFunctions::LinearInterpolation;
        }
    }

#if __has_include(<imgui/imgui.h>)
    // For ImVec4 animations
    inline ImVec4 operator*(ImVec4 const& lhs, float rhs) {
        return ImVec4{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
    }

    inline bool operator!=(ImColor const& lhs, ImColor const& rhs) {
        return ImVec4(lhs) != ImVec4(rhs);
    }
#endif

    enum class PlayPolicy {
        DEFAULT,
        ONLY_ONCE,
        ENDLESS
    };

    // Template Animation requires: operator!=(T, T), operator+(T, T), operator-(T, T), operator*(T, float).
    template <typename T>
    struct Animation {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<float>;
        using EasingFunc = std::function<float(float)>;

    public:
        T from = {};
        T to = {};
        Duration duration = std::chrono::milliseconds(250);
        Duration delay = std::chrono::milliseconds(0);
        EasingFunc easing_func = EasingFunctions::LinearInterpolation;
        bool reverse = false;
        bool* condition = nullptr;
        uint8_t loops = 0;
        PlayPolicy play_policy = PlayPolicy::DEFAULT;

        T value = from;
        Duration current{};
        float delta_time = 0.0167f;

    public:
        void update() {
            if (condition && !(*condition)) {
                if (value != from) {
                    current -= static_cast<Duration>(delta_time);
                    if (current.count() < 0) current = Duration(0);
                    float progress = (current.count() - delay.count()) / duration.count();
                    interpolate(progress);
                }
                return;
            }

            if (current < delay) {
                current += static_cast<Duration>(delta_time);
                return;
            }

            current += static_cast<Duration>(delta_time);

            if (current >= duration + delay) {
                if (reverse) {
                    std::swap(from, to);
                    current = Duration(0);

                    if (loops > 0) {
                        loops--;
                        if (loops == 0) reverse = false;
                    }
                }
                else if (loops > 0) {
                    current = Duration(0);
                    loops--;
                }
                else {
                    current = duration + delay;
                    value = to;
                    return;
                }
            }

            float progress = (current.count() - delay.count()) / duration.count();
            interpolate(progress);
        }

        void reset_duration() {
            current = Duration(0);
        }

        void reset_from(T new_from) {
            from = new_from;
            reset_duration();
        }

        void reset_to(T new_to) {
            from = value;
            to = new_to;
            reset_duration();
        }

        [[nodiscard]] bool finished() const {
            return current >= duration + delay;
        }

    private:
        void interpolate(float progress) {
            value = from + (to - from) * easing_func(progress);
        }
    };

    template <typename T>
    inline auto& get_active_anims() {
        static std::unordered_map<std::string, Animation<T>> active_anims;
        return active_anims;
    }

    template <typename T>
    inline void reset_anim(std::string name) {
        auto& active_anims = get_active_anims<T>();
        if (auto it = active_anims.find(name); it != active_anims.end()) {
            active_anims.erase(it);
        }
    }

    template <typename T>
    inline auto& get_once_anims() {
        static std::unordered_map<std::string, T> once_anims;
        return once_anims;
    }

    template <typename T>
    class AnimBuilder {
    private:
        Animation<T> animation_;
        std::function<void()> callback_;
        bool dynamic_change_range_ = true;
        bool no_startup_ = false;

    public:
        AnimBuilder& range(T from, T to) {
            animation_.from = from;
            animation_.to = to;
            return *this;
        }
        AnimBuilder& duration(typename Animation<T>::Duration duration) {
            animation_.duration = duration;
            return *this;
        }
        AnimBuilder& delay(typename Animation<T>::Duration delay) {
            animation_.delay = delay;
            return *this;
        }
        AnimBuilder& play_policy(PlayPolicy policy) {
            animation_.play_policy = policy;
            return *this;
        }
        AnimBuilder& easing(Easing easing) {
            animation_.easing_func = get_easing_func(easing);
            return *this;
        }
        AnimBuilder& condition(bool& condition) {
            animation_.condition = &condition;
            return *this;
        }
        AnimBuilder& reverse() {
            animation_.reverse = true;
            return *this;
        }
        AnimBuilder& no_startup() {
            no_startup_ = true;
            return *this;
        }
        AnimBuilder& loops(uint8_t loops) {
            animation_.loops = loops;
            return *this;
        }
        AnimBuilder& custom_easing(typename Animation<T>::EasingFunc func) {
            animation_.easing_func = func;
            return *this;
        }
        // This doesnt work with PlayPolicy::ENDLESS
        AnimBuilder& on_end(std::function<void()> func) {
            callback_ = func;
            return *this;
        }
        // Dynamic range only works with endless anims!
        // Means that if in range() passed new values that different than values passed before, 
        // animation is reset and interpolate based on new values.
        AnimBuilder& dynamic_range(bool dynamic_change_range) {
            dynamic_change_range_ = dynamic_change_range;
            return *this;
        }

        T build(const std::string& anim_name, float delta_time) {
            auto& active_anims = get_active_anims<T>();
            if (auto it = active_anims.find(anim_name); it != active_anims.end()) {
                auto& active_anim = it->second;

                active_anim.delta_time = delta_time;
                active_anim.condition = animation_.condition;

                // If animation is endless we can handle dynamic from/to range changes
                if (active_anim.play_policy == PlayPolicy::ENDLESS && dynamic_change_range_) {
                    //if (active_anim.from != animation_.from)
                        //active_anim.reset_from(animation_.from);
                    if (active_anim.to != animation_.to)
                        active_anim.reset_to(animation_.to);
                }
                active_anim.update();

                const T value = active_anim.value;
                if (active_anim.finished() && (animation_.play_policy != PlayPolicy::ENDLESS)) {
                    if (callback_)
                        callback_();

                    active_anims.erase(it);
                    if (animation_.play_policy == PlayPolicy::ONLY_ONCE)
                        get_once_anims<T>().emplace(anim_name, value);
                }
                return value;
            }
            else {
                const auto& once_anims = get_once_anims<T>();
                if (auto it = once_anims.find(anim_name); it != once_anims.end())
                    return it->second;

                const T start = animation_.from;
                animation_.delta_time = delta_time;
                if (no_startup_)
                    animation_.value = animation_.from;
                active_anims.emplace(anim_name, animation_);
                return start;
            }
        }
    };

    template<typename T>
    inline auto& get_anim_presets() {
        static std::unordered_map<std::string, AnimBuilder<T>> anim_presets;
        return anim_presets;
    }

    template<typename T>
    inline AnimBuilder<T> get_anim_preset(const std::string& preset_name) {
        const auto& anim_presets = get_anim_presets<T>();
        for (auto it = anim_presets.find(preset_name); it != anim_presets.end(); ++it) {
            return it->second;
        }
        throw std::runtime_error("ImAnim::get_anim_preset(): No animation preset with name " + preset_name);
    }

    template<typename T>
    inline void set_anim_preset(const std::string& name, AnimBuilder<T>&& builder) {
        auto& anim_presets = get_anim_presets<T>();
        anim_presets[name] = std::move(builder);
    }
}