#include <Zyphryon.Engine/Kernel.hpp>
#include <Zyphryon.Graphic/Service.hpp>

class Application final : public Engine::Kernel
{
public:

    Bool OnInitialize() override
    {
        return true;
    }

    void OnTick(Time Time) override
    {
        ConstTracker<Graphic::Service> Graphics = GetService<Graphic::Service>();

        const Graphic::Viewport Viewport(0, 0, GetDevice().GetWidth(), GetDevice().GetHeight());
        Graphics->Prepare(Graphic::kDisplay, Viewport, Graphic::Clear::All, Color::Black(), 1, 0);
        {

        }
        Graphics->Commit(Graphic::kDisplay);
    }

    void OnTeardown () override
    {

    }
};

int main()
{
    Engine::Properties Properties;
    Properties.SetWindowTitle("Zyphryon Example");
    Properties.SetWindowWidth(1024);
    Properties.SetWindowHeight(768);
    Properties.SetWindowFullscreen(false);
    Properties.SetWindowBorderless(false);
    Properties.SetVideoDriver("D3D11");
    Properties.SetAudioPauseOnFocusLost(true);

    Application Entry;
    Entry.Initialize(Service::Host::Mode::Client, Move(Properties));
    Entry.Run();

    return 1;
}
