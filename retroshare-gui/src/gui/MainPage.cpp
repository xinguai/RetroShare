#include <iostream>
#include <retroshare-gui/mainpage.h>
#include <QGraphicsBlurEffect>
#include <QAbstractButton>
#include <QGraphicsDropShadowEffect>

MainPage::MainPage(QWidget *parent , Qt::WindowFlags flags ) : QWidget(parent, flags) 
{
	help_browser = NULL ;
}

class MyTextBrowser: public QTextBrowser
{
	public:
		MyTextBrowser(QWidget *parent,QAbstractButton *bt)
			: QTextBrowser(parent),button(bt)
		{
		}

		virtual void	mousePressEvent ( QMouseEvent* )
		{
			hide() ;
			button->setChecked(false) ;
		}

	protected:
		QAbstractButton *button ;
};

void MainPage::showHelp(bool b) 
{
	help_browser->resize(size()*0.5) ;
	help_browser->move(width()/2 - help_browser->width()/2,height()/2 - help_browser->height()/2);
	help_browser->update() ;
	std::cerr << "Toggling help to " << b << std::endl;

	if(b)
		help_browser->show() ;
	else
		help_browser->hide() ;
}

void MainPage::registerHelpButton(QAbstractButton *button,const QString& help_html_txt)
{
	if(help_browser == NULL)
	{
		help_browser = new MyTextBrowser(this,button) ;

		QGraphicsDropShadowEffect * effect = new QGraphicsDropShadowEffect(help_browser) ;
		effect->setBlurRadius(30.0);
		help_browser->setGraphicsEffect(effect);

		help_browser->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum)) ;
		help_browser->hide() ;
	}

	help_browser->setHtml("<div align=\"justify\">"+help_html_txt+"</div>") ;

	QObject::connect(button,SIGNAL(toggled(bool)), this, SLOT( showHelp(bool) ) ) ;
}

