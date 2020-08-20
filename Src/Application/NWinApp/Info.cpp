#include "Info.h"

#define APP_SRAM_ADDRESS			0x20000000
#define APP_SRAM_SIZE				0x00002800							// 10240 B = 10 KB

extern "C"
{
	extern unsigned int _end_bss;
};

static bool sbStop;

static void OnClose(bool &bCanClose)
{
	DBG_PRINTF("%s: Stop info window.\r\n", __FUNCTION__);
	sbStop = true;
}

Info::Info()
{
	m_Wind.Text = "Information";
}

Info::~Info()
{
}

void Info::_HandleButtonTouched()
{
	if (m_lbLogo.Visible) {
		m_lbLogo.Visible = false;
		m_btnShow.Text = "Show Logo";
	} else {
		m_lbLogo.Visible = true;
		m_btnShow.Text = "Hide Logo";
	}
}

bool Info::Show()
{
	Rectangle rc;
	bool b = false;
	EventHandler<>	Handler;

	do {
		b = m_Wind.Create();
		if (!b) break;

		m_Wind.OnClose = OnClose;

		m_Wind.GetClientRectangle(rc);
		rc += -5;
		rc.Bottom = rc.Top + 20;

		b = m_lbOSName.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbOSName.Text = "OS Name : NanoOS";

		rc.Offset(0, 20);
		b = m_lbOSVer.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbOSVer.Text = "OS Version : 1.2";

		rc.Offset(0, 20);
		b = m_lbSpeed.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbSpeed.Text = "Clock Speed : 72 MHz.";

		rc.Offset(0, 20);
		b = m_lbChipset.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbChipset.Text = "Chipset : STM32F103XXX";

		rc.Offset(0, 20);
		b = m_lbMaxThd.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbMaxThd.Text = "Max. threads : 4";

		rc.Offset(0, 20);
		b = m_lbMem.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbMem.Text = "App. SRAM size : 10 KB.";

		rc.Offset(0, 30);
		b = m_btnShow.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_btnShow.Text = "Show Logo";
		Handler.m_pReceiver = dynamic_cast<EventReceiver *>(this);
		Handler.m_pHandlerNoArg = reinterpret_cast<void (EventReceiver:: *)()>(&Info::_HandleButtonTouched);
		m_btnShow.OnTouched = Handler;

		rc.Offset(0, 40);
		rc.Bottom += 60;
		b = m_lbLogo.Create(rc.Left, rc.Top, rc.Width(), rc.Height(), m_Wind);
		if (!b) break;
		m_lbLogo.Visible = false;
		m_lbLogo.MultiLine = false;
		m_lbLogo.TextScale = 2;
		m_lbLogo.Border = true;
		m_lbLogo.Align = TextAlign::Center;
		m_lbLogo.Text = "NanoOS";

	} while (false);

	if (!b) {
		DBG_PRINTF("%s: Error!\r\n", __FUNCTION__);
		m_Wind.Destroy();
		return false;
	} else {
		sbStop = false;
		m_Wind.Show();
		while (!sbStop) {
			b = m_Wind.GetInput(m_Input);
			if (b) {
				m_Wind.ProcessInput(m_Input);
			}
		}
	}

	m_Wind.Destroy();

	return b;
}