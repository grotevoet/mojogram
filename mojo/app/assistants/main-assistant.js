function MainAssistant() {
    /* this is the creator function for your scene assistant object. It will be passed all the
     additional parameters (after the scene name) that were passed to pushScene. The reference
     to the scene controller (this.controller) has not be established yet, so any initialization
     that needs the scene controller should be done in the setup function below. */

    this.contactsImported = false;
    this.chatsLoaded = false;

    _mainAssistant = this;
    this.nunread = 0;

    this.mainListModel = {
        items : [{
            name : $L("Chats"),
            icon : "Chats",
            size : 0,
            unread : false,
            chats : [],
            nunread : ""
        }, {
            name : $L("Contacts"),
            icon : "Contacts",
            size : 0,
            contacts : [],
            unread : false
        }, {
            name : $L("Preferences"),
            icon : "Preferences",
            size : "",
            unread : false
        }
        // {name: "Favourite Cont", size: 0}

        ]
    };
}

MainAssistant.prototype.setup = function() {
    PalmServices.setWakeUpAlarm();
    PalmServices.setActivity();

    this.controller.setupWidget("spinnerId", this.attributesSpinner = {
        spinnerSize : "large"
    }, this.modelSpinner = {
        spinning : true
    });

    // Start pluginBG.
    _mojowhatsupPlugin.safePluginCall(function() {
         delete _mediaUploads;
         delete _mediaDownloads;
         _mediaUploads = new HashTable();
         _mediaDownloads = new HashTable();
        
        _myJid = _appData.cookieData.userId + "@s.whatsapp.net";    	
        _plugin.startBG(_appData.cookieData.userId, _appData.cookieData.password, _appData.cookieData.pushName);
    });
    
    _mojowhatsupPlugin.whenRunnerExecuting( function() {
        PalmServices.subscribeNetworkStatus(this.controller);
    }.bind(this));

    this.controller.setupWidget(Mojo.Menu.appMenu, this.attributesAppMenu = {
        omitDefaultItems: true
    }, this.appMenuModel = {
        visible: true,
        items : [
            Mojo.Menu.editItem,
            {label: $L("Preferences"), command: Mojo.Menu.prefsCmd},
            {label: $L("Account settings"), command: "accountSettings"},
            {label: $L("Help"), command: Mojo.Menu.helpCmd},
            {label: $L("Exit"), command: "exit"}
        ]
    });

    this.controller.get("headerTitle").update($L("Main Menu"));

    this.mainListElement = this.controller.get("mainList");

    this.controller.setupWidget("mainList", {
        itemTemplate : "main/menu-entry",

    }, this.mainListModel);

    /* add event handlers to listen to events from widgets */

    this.listTapHandler = this.listTapHandler.bindAsEventListener(this);
    Mojo.Event.listen(this.mainListElement, Mojo.Event.listTap, this.listTapHandler);

    if (!_contactsImported) {
        _appDB.importContacts(this.controller, function() {
            _contactsImported = true;
        });
    }

    this.loadContacts();
    this.updateChats();
    this.waitForCompletion();
};

MainAssistant.prototype.restart = function() {
    _mojowhatsupPlugin.safePluginCall(function() {
        _plugin.startBG(_appData.cookieData.userId, _appData.cookieData.password, _appData.cookieData.pushName);
    });
}

MainAssistant.prototype.loadContacts = function(callback) {
    if (_contactsImported) {
        _appDB.getAllContacts( function(contacts) {
            this.mainListModel.items[1].size = contacts.length;
            this.mainListModel.items[1].contacts = contacts;
            this.controller.modelChanged(this.mainListModel);
            this.contactsImported = true;
			if (callback)
				callback(contacts);            
        }.bind(this));
    } else {
        setTimeout(this.loadContacts.bind(this), 10, callback);
    }
}

MainAssistant.prototype.waitForCompletion = function() {
    if (_mojowhatsupPlugin.isRunnerExecuting && this.chatsLoaded && this.contactsImported) {
        this.controller.get("spinnerId").mojo.stop();
        this.controller.get("Scrim").hide();
        _contactJidNames.setItem(_myJid, $L("You"));     
    } else {
        setTimeout(this.waitForCompletion.bind(this), 30);
    }
}

MainAssistant.prototype.listTapHandler = function(event) {
    if (event.index === 1)
        this.controller.stageController.pushScene("contact-list", event.item.contacts);
    else if (event.index === 0)
        this.controller.stageController.pushScene("chats-list", event.item.chats);
    else if (event.index === 2)
        this.controller.stageController.pushScene("prefs");
}

MainAssistant.prototype.activate = function(event) {
    /* put in event handlers here that should only be in effect when this scene is active. For
     example, key handlers that are observing the document */
};

MainAssistant.prototype.deactivate = function(event) {
    /* remove any event handlers you added in activate and do any other cleanup that should happen before
     this scene is popped or another scene is pushed on top */
};

MainAssistant.prototype.cleanup = function(event) {
    PalmServices.clearWakeUpAlarm();
    PalmServices.clearActivity();

    _mainAssistant = null;
    Mojo.Event.stopListening(this.mainListElement, Mojo.Event.listTap, this.listTapHandler);
    _mojowhatsupPlugin.safePluginCall(function() {
        _plugin.exitPlugin();
    });
    if (!_exitApp) {
    _appAssistant.handleLaunch({
        action : "dashboard"
    });
    }
};

MainAssistant.prototype.handleCommand = function(event) {
    if(event.type == Mojo.Event.commandEnable && (event.command == Mojo.Menu.helpCmd || event.command == Mojo.Menu.prefsCmd)) {
      event.stopPropagation();
    }    
    if (event.type == Mojo.Event.command) {
        switch(event.command) {
            case Mojo.Menu.prefsCmd:
                this.controller.stageController.pushScene("prefs");
            break;
            case "accountSettings":
                this.controller.stageController.pushScene("account");
            break;
            case Mojo.Menu.helpCmd:
                this.controller.stageController.pushAppSupportInfoScene();
                break;
            case "exit":
                 _exitApp = true;
                 Mojo.Controller.getAppController().closeStage(_mainStage);
            break;
        }
    }
}

MainAssistant.prototype.updateChats = function() {
    _appDB.getAllChatsWithMessages( function(chats) {
        this.nunread = 0;
        for (var i = 0; i < chats.length; i++)
            this.nunread += chats[i].unread;

        this.mainListModel.items[0].size = chats.length;
        this.mainListModel.items[0].chats = chats;
        this.mainListModel.items[0].nunread = (this.nunread == 0 ? "" : this.nunread);
        this.mainListModel.items[0].unread = "false";

        if (this.nunread > 0)
            this.mainListModel.items[0].unread = "true";

        this.controller.modelChanged(this.mainListModel);
        this.chatsLoaded = true;
    }.bind(this));
}