#include <QFile>
#include <QDir>
#include <QRegExp>

#include <QDebug>

#include <QWidget> //a strange thing: it compiles without this header, but 
                  //then segfaults in some place

#include <QApplication>

#include "PluginManager.h"
#include "PluginManagerWidget.h"
#include "plugins/PluginInterface.h"
#include "rshare.h"

//=============================================================================
//=============================================================================
//=============================================================================

PluginManager::PluginManager()
{
    baseFolder =  //qApp->applicationDirPath()+"///plugins" ;
Rshare::dataDirectory() + "/plugins" ;
    lastError = "No error.";

    viewWidget = 0;

//    defaultLoad();
}

//=============================================================================
      
void      
PluginManager::defaultLoad(  )
{   
   qDebug() << "  " << "Default load started" ;

    QDir workDir(baseFolder);

    if ( !workDir.exists() )
    {
        QString em= QString("base folder %1 doesn't exist, default load failed")
	              .arg( baseFolder );
        emit errorAppeared( em ); 
	return ;
    }

  //=== get current available plugins =====
    QStringList currAvailable = workDir.entryList(QDir::Files);
#if defined(Q_OS_WIN)    
    QRegExp trx("*.dll") ;
#else
    QRegExp trx("*.so");
#endif
    trx.setPatternSyntax(QRegExp::Wildcard );

    currAvailable.filter( trx );

    qDebug() << "  " << "can load this plugins: " << currAvailable ;

  //=== 
    foreach(QString pluginFileName, currAvailable)
    {
       QString fullfn( workDir.absoluteFilePath( pluginFileName ) );
       QString newName;
       int ti = readPluginInformation( fullfn, newName);
       if (! ti )
       {
           acceptPlugin(fullfn, newName);
       }
    }//    foreach(QString pluginFileName, currAvailable)

    qDebug() << "  " << "names are " << names;
}

//=============================================================================

int
PluginManager::readPluginInformation(QString fullFileName, QString& pluginName)
{
    qDebug() << "  " << "processing file " << fullFileName;

    PluginInterface* plugin = loadPluginInterface(fullFileName) ;
    pluginName = "Undefined name" ;
    if (plugin)
    {
        pluginName = plugin->pluginName();
        qDebug() << "  " << "got pluginName:"  << pluginName;
        delete plugin;
        return 0 ;
    }
    else
    {
       //do not emit anything, cuz error message already was sent 
      //from loadPluginInterface(..)
        return 1; //this means, some rrror appeared
    }
}

//=============================================================================

PluginInterface* 
PluginManager::loadPluginInterface(QString fileName)
{
    QString errorMessage = "Default Error Message" ;
    PluginInterface* plugin = 0 ;
    QPluginLoader* plLoader = new QPluginLoader(fileName);

    QObject *pluginObject = plLoader->instance();
    if (pluginObject)
    {
        //qDebug() << "  " << "loaded..." ;
        plugin = qobject_cast<PluginInterface*> (pluginObject) ;
	   
        if (plugin)
        {
            errorMessage = "No error" ;            
        }
        else
        {
            errorMessage = "Cast to 'PluginInterface*' failed";         
            emit errorAppeared( errorMessage );
        }        
    }
    else
    {
        errorMessage = "Istance wasn't created: " + plLoader->errorString() ;
        emit errorAppeared( errorMessage );
    }

    delete plLoader; // plugin instance will not be deleted with this action
    return plugin;
}

//=============================================================================

PluginManager:: ~PluginManager()
{   
}

//=============================================================================

void 
PluginManager::acceptPlugin(QString fileName, QString pluginName)
{
    qDebug() << "  " << "accepting plugin " << pluginName;

    names.append(pluginName);
    fileNames.append(fileName);

    if (viewWidget)
       viewWidget->registerNewPlugin( pluginName );

    emit newPluginRegistered( pluginName );
}

//=============================================================================

QWidget* 
PluginManager::pluginWidget(QString pluginName)
{
    QWidget* result = 0;
    int plIndex = names.indexOf( pluginName ) ;
    if (plIndex >=0 )
    {
    //=== load plugin's interface
        QString fn = fileNames.at(plIndex);
        PluginInterface* pliface = loadPluginInterface(fn);
        if (pliface)
        {
       //=== now, get a widget
            result = pliface->pluginWidget() ;
            if (result)
            {
             // all was good,
                qDebug() << "  " << "got plg widget..." ;
		return result;
            }
            else
            {
                QString em=QString("Error: instance '%1'can't create a widget")
                            .arg( pluginName );
                emit errorAppeared( em );
		return 0;
            }
        }
        else
        {
            // do nothing here...
        }
    }
    else
    {
        QString em = QString("Error: no plugin with name '%1' found")
                        .arg(pluginName);
        emit errorAppeared( em );
    }      
}

//=============================================================================

QWidget* 
PluginManager::getViewWidget(QWidget* parent )
{
    if (viewWidget)
        return viewWidget;
    
  //=== else, create the viewWidget and return it
  //
    viewWidget = new PluginManagerWidget();
    
    foreach(QString pn, names)
    {
       qDebug() << "  " << "reg new plg " << pn;
       viewWidget->registerNewPlugin( pn );
    }
    
    connect(this      , SIGNAL( errorAppeared(QString) )    ,
            viewWidget, SLOT( acceptErrorMessage( QString)));
    
    connect(viewWidget, SIGNAL( destroyed() )    ,
            this      , SLOT( viewWidgetDestroyed( )));

    connect(viewWidget, SIGNAL( installPluginRequested(QString)),
	    this      , SLOT(   installPlugin( QString)));

    connect(viewWidget, SIGNAL( removeRequested( QString ) ),
	    this      , SLOT(   removePlugin(QString )));
	    
    


    qDebug() << "  PluginManager::getViewWidget done";

    return viewWidget;
}

//=============================================================================

void
PluginManager::viewWidgetDestroyed(QObject* obj )
{ 
    qDebug() << "  PluginManager::viewWidgetDestroyed is here";
    
    viewWidget = 0;
}

//=============================================================================

void 
PluginManager::removePlugin(QString pluginName)
{
    QWidget* result = 0;
    int plIndex = names.indexOf( pluginName ) ;
    if (plIndex >=0 )
    {
        QString fn = fileNames.at(plIndex);
        if (QDir::isRelativePath(fn))
	    fn = QDir(baseFolder).path() + QDir::separator() + fn ;

        QFile fl(fn);
        if (!fl.remove())
        {
	    QString em = QString("Error: failed to revove file %1"
	                         "(uninstalling plugin '%2')")
                            .arg(fn).arg(pluginName);
            emit errorAppeared( em);
        }
        else
        {
            if (viewWidget)
	        viewWidget->removePluginFrame( pluginName);
	}
    }
    else
    {
       QString em = QString("Error(uninstall): no plugin with name '%1' found")
		      .arg(pluginName);
       emit errorAppeared( em );
    }
}

//=============================================================================

void
PluginManager::installPlugin(QString fileName)
{
    qDebug() << "  " << "PluginManager::installPlugin is here" ;
   
    if (!QFile::exists( fileName) )
    {       
       QString em = QString("Error(installation): flugin file %1 doesn't exist")
 		      .arg( fileName );

       emit errorAppeared( em );
       return ;
    }

    QString pn;
    if (! readPluginInformation(fileName, pn))
    {
        QFile sf( fileName) ;
        QString newFileName = baseFolder + QDir::separator() +
	                      QFileInfo( fileName).fileName();
        if ( QFile::copy( fileName, newFileName ) )
        {
            QString pn;
            int ti = readPluginInformation( newFileName , pn );
            if (! ti )
            {
                acceptPlugin(newFileName, pn);
            }
        } //    
	else
	{
	    QString em = QString("Error: can't copy %1 as %2")
	                    .arg(fileName, newFileName) ;
            emit errorAppeared( em );
	}
    }
    else
    {
//       do nothing here: read plugin information emits its own error
    }
}

//=============================================================================

