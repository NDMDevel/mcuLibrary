#pragma once

#include <array>
#include <cstdint>

/**
 * fs [Hz]: sampling frequency
 *
 * min_period [seconds]: any period smaller than this value will be considered
 *             as noise.
 */
template<float fs,float min_period>
class zcd_t
{
    static_assert(fs > 0,
                  "fs (sampling freq) must be greater than zero");
    static_assert(min_period > 0,
                  "min_period must be greater than zero");
    static constexpr uint32_t _min_samples_per_period = fs*min_period/2.0f  ;
public:
    void process_sample(float sample)
    {
        _zcd_global_counter++;
        //detect zc
        if( _prev_sample * sample < 0.0f )
        {
            if( !zc_noisy() || _zc_cnt < _from_zc_cnt.size() )
            {
                //zc detected
                _zcd_global_counter++;
                if( _zc_cnt < _from_zc_cnt.size() )
                    _zc_cnt++;

                //compute zc time:
                float m  = (sample-_prev_sample)*fs;
                float zc_dt = -sample / m;
                //compute period
                auto half_period = float(_from_zc_cnt[2])*1.0f/fs + zc_dt - _prev_zc_dt;
                _prev_zc_dt = zc_dt;

                _half_period[0] = _half_period[1];
                _half_period[1] = half_period;

                _from_zc_cnt[0] = _from_zc_cnt[1];
                _from_zc_cnt[1] = _from_zc_cnt[2];
                _from_zc_cnt[2] = 0;

//                if( _zc_cnt >= _from_zc_cnt.size() )
                if( _zcd_callback != nullptr )
                    _zcd_callback();
                if( _zcd_md_callback != nullptr )
                    _zcd_md_callback(_zcd_global_counter,_samples_global_counter);
            }
        }//end zc detection
        if( _zc_cnt != 0 )
            _from_zc_cnt[2]++;
        _prev_sample = sample;
    }
    std::pair<float,float> get_half_period() const { return {_half_period[0],_half_period[1]}; }
    void install_zcd_callback(void(*callback)()) { _zcd_callback = callback; }
    void install_zcd_metadata_callback(void(*callback)(uint32_t,uint32_t)) { _zcd_md_callback = callback; }
protected:
    bool zc_noisy() const
    {
        if( _from_zc_cnt[2] < std::min(_from_zc_cnt[0],_from_zc_cnt[1])/4 ||
            _from_zc_cnt[2] < _min_samples_per_period/2 )
            return true;
        return false;
    }
private:
    std::array<uint32_t,3>  _from_zc_cnt;
    float                   _prev_sample = 0;
    float                   _prev_zc_dt  = 0;
    std::array<float,2>     _half_period;
    void(*_zcd_callback)()                          = nullptr;
    void(*_zcd_md_callback)(uint32_t,uint32_t)      = nullptr;
    uint32_t                _zcd_global_counter     = 0;
    uint32_t                _samples_global_counter = 0;
    uint8_t  _zc_cnt = 0;
};

