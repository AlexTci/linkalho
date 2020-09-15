#include "worker_page.hpp"
#include <borealis.hpp>
#include <math.h>
#include "progress_event.hpp"
#include <functional>

WorkerPage::WorkerPage(brls::StagedAppletFrame* frame, const std::string& warning, worker_func_t worker_func): frame(frame), workerFunc(worker_func)
{

    this->progressDisp = new brls::ProgressDisplay();
    this->progressDisp->setParent(this);

    this->label = new brls::Label(brls::LabelStyle::DIALOG, warning, true);
    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    this->button = new brls::Button(brls::ButtonStyle::BORDERLESS);  // avoid back button bug
    this->button->setParent(this);
}

void WorkerPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    if (!this->workStarted)
    {
        this->workStarted = true;
        workerThread = new std::thread(&WorkerPage::doWork, this);
    }
    else if (ProgressEvent::instance().finished())
    {
        frame->nextStage();
    }
    else
    {
        this->progressDisp->setProgress(ProgressEvent::instance().increment(), ProgressEvent::instance().getMax());
        this->progressDisp->frame(ctx);

        this->label->frame(ctx);
    }
}

void WorkerPage::doWork()
{
    if (this->workerFunc)
        this->workerFunc();
    //frame->nextStage();
}

brls::View* WorkerPage::getDefaultFocus()
{
    return this->button;
}

void WorkerPage::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->label->setWidth(roundf((float)this->width * style->CrashFrame.labelWidth));
    //this->label->invalidate(true);

    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - style->AppletFrame.footerHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight());

    this->progressDisp->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth,
        this->y + this->height / 2,
        style->CrashFrame.buttonWidth * 2,
        style->CrashFrame.buttonHeight);

    // this->button->setBoundaries(
    //     this->x,
    //     this->y,
    //     0,
    //     0);
}

void WorkerPage::willAppear(bool resetState)
{
    this->progressDisp->willAppear(resetState);
}

void WorkerPage::willDisappear(bool resetState)
{
    this->progressDisp->willDisappear(resetState);
}

WorkerPage::~WorkerPage()
{
    if (this->workStarted && workerThread->joinable())
    {
        this->workerThread->join();
        if (this->workerThread)
            delete this->workerThread;
    }

    delete this->progressDisp;
    delete this->label;
    delete this->button;
}
