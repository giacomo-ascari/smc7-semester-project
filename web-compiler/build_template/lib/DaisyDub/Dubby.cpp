#include "Dubby.h"

using namespace daisy;

// Hardware Definitions
#define PIN_GATE_IN_1 32
#define PIN_GATE_IN_2 23
#define PIN_GATE_IN_3 17
#define PIN_GATE_IN_4 19
#define PIN_KNOB_1 18
#define PIN_KNOB_2 15
#define PIN_KNOB_3 21
#define PIN_KNOB_4 22
#define PIN_JS_CLICK 3
#define PIN_JS_V 20
#define PIN_JS_H 16
#define PIN_ENC_CLICK 4
#define PIN_ENC_A 6
#define PIN_ENC_B 5 
#define PIN_OLED_DC 9
#define PIN_OLED_RESET 31

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define PANE_X_START 1
#define PANE_X_END 126
#define PANE_Y_START 1
#define PANE_Y_END 51

#define SUBMENU_X_START 1
#define SUBMENU_X_END 63
#define SUBMENU_Y_START 1
#define SUBMENU_Y_END 11

void Dubby::Init() 
{
    InitControls();
    InitGates();
    
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();

    for (int i = 0; i < 5; i++) 
    {
        submenuBoxBounding[i][0] = SUBMENU_X_START;
        submenuBoxBounding[i][1] = SUBMENU_Y_START + i * 10;
        submenuBoxBounding[i][2] = SUBMENU_X_END;
        submenuBoxBounding[i][3] = SUBMENU_Y_END + i * 10;
    }

    InitDisplay();
    InitEncoder();
    InitAudio();
}

void Dubby::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle(seed.GetPin(PIN_KNOB_1));
    cfg[CTRL_2].InitSingle(seed.GetPin(PIN_KNOB_2));
    cfg[CTRL_3].InitSingle(seed.GetPin(PIN_KNOB_3));
    cfg[CTRL_4].InitSingle(seed.GetPin(PIN_KNOB_4));
    cfg[CTRL_5].InitSingle(seed.GetPin(PIN_JS_H));
    cfg[CTRL_6].InitSingle(seed.GetPin(PIN_JS_V));

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize analogInputs, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        analogInputs[i].Init(seed.adc.GetPtr(i), seed.AudioCallbackRate(), true);
    }

    seed.adc.Start();
}

void Dubby::InitGates()
{
    dsy_gpio_pin pin;
    pin = seed.GetPin(PIN_GATE_IN_1);
    gateInputs[GATE_IN_1].Init(&pin);
    pin = seed.GetPin(PIN_GATE_IN_2);
    gateInputs[GATE_IN_2].Init(&pin);
    pin = seed.GetPin(PIN_GATE_IN_3);
    gateInputs[GATE_IN_3].Init(&pin);
    pin = seed.GetPin(PIN_GATE_IN_4);
    gateInputs[GATE_IN_4].Init(&pin);
    pin = seed.GetPin(PIN_JS_CLICK);
    gateInputs[GATE_IN_5].Init(&pin);
}


void Dubby::InitDisplay() 
{
    /** Configure the Display */
    OledDisplay<SSD130x4WireSpi128x64Driver>::Config disp_cfg;
    disp_cfg.driver_config.transport_config.pin_config.dc    = seed.GetPin(9);
    disp_cfg.driver_config.transport_config.pin_config.reset = seed.GetPin(31);
    /** And Initialize */
    display.Init(disp_cfg);
}

void Dubby::UpdateDisplay() 
{
    if (encoder.TimeHeldMs() > 300) 
    {
        if (!menuActive) 
        {
            HightlightMenuItem();
            menuActive = true;
        }

        if (encoder.Increment()) UpdateMenu(encoder.Increment(), true);
    } 

    switch(menuItemSelected) 
    {
        case MENU1:
            RenderScope();
            break;
        case MENU2:
            UpdateMixerPane();
            break;
        case MENU3:
            if (encoder.Pressed() && preferencesMenuItemSelected == OPTION4) ResetToBootloader();
            if (encoder.Increment() && !menuActive) UpdatePreferencesMenu(encoder.Increment());
            break;
        default:
            break;
    }
    
    if (encoder.TimeHeldMs() < 300 && menuActive)
    {
        menuActive = false;
        
        ReleaseMenu();
        UpdateSubmenu();
    }
}

void Dubby::DrawLogo() 
{
    DrawBitmap(0);   
}

void Dubby::DrawBitmap(int bitmapIndex)
{
    display.Fill(false);
    display.SetCursor(30, 30);
    
    for (int x = 0; x < OLED_WIDTH; x += 8) {
        for (int y = 0; y < OLED_HEIGHT; y++) {
            // Calculate the index in the array
            int byteIndex = (y * OLED_WIDTH + x) / 8;

            // Get the byte containing 8 pixels
            char byte = bitmaps[bitmapIndex][byteIndex];

            // Process 8 pixels in the byte
            for (int bitIndex = 0; bitIndex < 8; bitIndex++) {

                // Get the pixel value (0 or 1)
                char pixel = (byte >> (7 - bitIndex)) & 0x01;

                bool isWhite = (pixel == 1);

                display.DrawPixel(x + bitIndex, y, isWhite);
            }

            if (y % 6 == 0) display.Update();
        }
    }
}

void Dubby::UpdateMenu(int increment, bool higlight) 
{
    if ((menuItemSelected >= 0 && increment == 1 && menuItemSelected < 2) || (increment != 1 && menuItemSelected != 0))
        menuItemSelected = (MenuItems)(menuItemSelected + increment);

    display.Fill(false);

    if (higlight) HightlightMenuItem();
    else ReleaseMenu();
    
    UpdateSubmenu();

    display.Update();
}

void Dubby::HightlightMenuItem() 
{
    display.DrawRect(menuBoxBounding[menuItemSelected][0], menuBoxBounding[menuItemSelected][1], menuBoxBounding[menuItemSelected][2], menuBoxBounding[menuItemSelected][3], true, true);

    for (int i = 0; i < 3; i++) 
    {
        display.SetCursor(menuTextCursors[i][0], menuTextCursors[i][1]);
        display.WriteString(GetTextForEnum(MAINMENU, i), Font_6x8, i == menuItemSelected ? false : true);
    }

    display.DrawRect(PANE_X_START - 1, PANE_Y_START - 1, PANE_X_END + 1, PANE_Y_END + 1, true);

    display.Update();
}

void Dubby::ReleaseMenu() 
{        
    display.Fill(false);
    display.DrawRect(menuBoxBounding[menuItemSelected][0], menuBoxBounding[menuItemSelected][1], menuBoxBounding[menuItemSelected][2], menuBoxBounding[menuItemSelected][3],true);

    for (int i = 0; i < MENU_LAST; i++) 
    {
        display.SetCursor(menuTextCursors[i][0], menuTextCursors[i][1]);
        display.WriteString(GetTextForEnum(MAINMENU, i), Font_6x8, true);
    }

    display.Update();
}

void Dubby::UpdateMixerPane() 
{
    if(seed.system.GetNow() - screen_update_last_ > screen_update_period_)
    {
        screen_update_last_ = seed.system.GetNow();
        for (int i = 0; i < 4; i++) UpdateBar(i);
    }
}

void Dubby::UpdateSubmenu()
{
    switch(menuItemSelected) 
    {
        case MENU1:
            RenderScope();
            break;
        case MENU2:
            // for (int i = 0; i < 4; i++) UpdateBar(i);
            break;
        case MENU3:
            DisplayPreferencesMenu();
            break;
        default:
            break;
    }

    display.Update();
}

void Dubby::UpdateBar(int i) 
{
    display.DrawRect((i * 32)  + margin, 1, ((i + 1) * 31) - margin, 51, false, true);
    display.DrawRect((i * 32)  + margin, int(abs(1.0f - GetKnobValue(static_cast<Dubby::Ctrl>(i))) * 51.0f) + 1, ((i + 1) * 31) - margin, 51, true, false);
    display.DrawRect((i * 32)  + margin, int(abs((currentLevels[i] * 5.0f) - 1.0f) * 51.0f) + 1, ((i + 1) * 31) - margin, 51, true, true);
    
    display.Update();
}

void Dubby::RenderScope()
{
    if(seed.system.GetNow() - screen_update_last_ > screen_update_period_)
    {
        screen_update_last_ = seed.system.GetNow();
        display.DrawRect(PANE_X_START, PANE_Y_START, PANE_X_END, PANE_Y_END, false, true);
        int prev_x = 0;
        int prev_y = (OLED_HEIGHT - 15) / 2;
        for(size_t i = 0; i < AUDIO_BLOCK_SIZE; i++)
        {
            int y = 1 + std::min(std::max((OLED_HEIGHT - 15) / 2
                                        - int(scope_buffer[i] * 150),
                                    0),
                            OLED_HEIGHT - 15);
            int x = 1 + i * (OLED_WIDTH - 2) / AUDIO_BLOCK_SIZE;
            if(i != 0)
            {
                display.DrawLine(prev_x, prev_y, x, y, true);
            }
            prev_x = x;
            prev_y = y;
        }

        display.Update();
    }   
}

void Dubby::DisplayPreferencesMenu()
{
    display.DrawRect(PANE_X_START, PANE_Y_START, PANE_X_END, PANE_Y_END, false, true);

    for (int i = 0; i < PREFERENCESMENU_LAST; i++)
    {
        display.SetCursor(5, 3 + (i * 10));
        display.WriteString(GetTextForEnum(PREFERENCESMENU, i), Font_6x8, true);

        if (preferencesMenuItemSelected == i)
            display.DrawRect(submenuBoxBounding[i][0], submenuBoxBounding[i][1], submenuBoxBounding[i][2], submenuBoxBounding[i][3], true);
    }

    display.Update();
}

void Dubby::UpdatePreferencesMenu(int increment) 
{
    if (((preferencesMenuItemSelected >= 0 && increment == 1 && preferencesMenuItemSelected < PREFERENCESMENU_LAST - 1) || (increment != 1 && preferencesMenuItemSelected != 0))) 
    {
        preferencesMenuItemSelected = (PreferenesMenuItems)(preferencesMenuItemSelected + increment);
        
        DisplayPreferencesMenu();
    }
}

void Dubby::ResetToBootloader() 
{
    DrawBitmap(1);

    display.SetCursor(20, 55);
    display.WriteString("FIRMWARE UPDATE", Font_6x8, true);

    display.Update();

    System::ResetToBootloader();
}

void Dubby::InitEncoder()
{
    encoder.Init(seed.GetPin(PIN_ENC_A),
                seed.GetPin(PIN_ENC_B),
                seed.GetPin(PIN_ENC_CLICK));
}

void Dubby::ProcessAllControls()
{
    ProcessAnalogControls();
    ProcessDigitalControls();
}

void Dubby::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
        analogInputs[i].Process();
}

void Dubby::ProcessDigitalControls()
{
    encoder.Debounce();
}

float Dubby::GetKnobValue(Ctrl k)
{
    return (analogInputs[k].Value());
}

void Dubby::InitAudio() 
{
    // Handle Seed Audio as-is and then
    SaiHandle::Config sai_config[2];
    // Internal Codec
    if(seed.CheckBoardVersion() == DaisySeed::BoardVersion::DAISY_SEED_1_1)
    {
        sai_config[0].pin_config.sa = {DSY_GPIOE, 6};
        sai_config[0].pin_config.sb = {DSY_GPIOE, 3};
        sai_config[0].a_dir         = SaiHandle::Config::Direction::RECEIVE;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::TRANSMIT;
    }
    else
    {
        sai_config[0].pin_config.sa = {DSY_GPIOE, 6};
        sai_config[0].pin_config.sb = {DSY_GPIOE, 3};
        sai_config[0].a_dir         = SaiHandle::Config::Direction::TRANSMIT;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::RECEIVE;
    }
    sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[0].pin_config.fs   = {DSY_GPIOE, 4};
    sai_config[0].pin_config.mclk = {DSY_GPIOE, 2};
    sai_config[0].pin_config.sck  = {DSY_GPIOE, 5};

    // External Codec

    I2CHandle::Config i2c_cfg;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = {DSY_GPIOB, 8};
    i2c_cfg.pin_config.sda = {DSY_GPIOB, 9};

    I2CHandle i2c2;
    i2c2.Init(i2c_cfg);

    // pullups must be enabled
    GPIOB->PUPDR &= ~((GPIO_PUPDR_PUPD8)|(GPIO_PUPDR_PUPD9)); 
    GPIOB->PUPDR |= ((GPIO_PUPDR_PUPD8_0)|(GPIO_PUPDR_PUPD9_0)); 

    Pcm3060 codec;

    codec.Init(i2c2);        

    sai_config[1].periph          = SaiHandle::Config::Peripheral::SAI_2;
    sai_config[1].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[1].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[1].b_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[1].pin_config.fs   = {DSY_GPIOG, 9};
    sai_config[1].pin_config.mclk = {DSY_GPIOA, 1};
    sai_config[1].pin_config.sck  = {DSY_GPIOA, 2};
    sai_config[1].a_dir         = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[1].pin_config.sa = {DSY_GPIOD, 11};
    sai_config[1].b_dir         = SaiHandle::Config::Direction::RECEIVE;
    sai_config[1].pin_config.sb = {DSY_GPIOA, 0};

    SaiHandle sai_handle[2];
    sai_handle[0].Init(sai_config[1]);
    sai_handle[1].Init(sai_config[0]);

    // Reinit Audio for _both_ codecs...
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f;
    seed.audio_handle.Init(cfg, sai_handle[0], sai_handle[1]);
}

const char * Dubby::GetTextForEnum(MenuTypes m, int enumVal)
{
    switch (m)
    {
        case MAINMENU:
            return MenuItemsStrings[enumVal];
            break;
        case PREFERENCESMENU:
            return PreferencesMenuItemsStrings[enumVal];
            break;
        default:
            return "";
            break;
    }
}