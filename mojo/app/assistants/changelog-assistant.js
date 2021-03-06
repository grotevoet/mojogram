function ChangelogAssistant() {
	this.changelog = [
	    {
            version: "v1.6.2",
            changes: [
				"GUI simplification (removed main-menu scene)",
				"Solved the problem of image preview",
				"Solved the problem of unread message display",
				"Choose between Palm or Whatsapp speech bubles in Preferences",
				"Open a private chat with a group participant tapping on his/her message",
				"Solved some other minor bugs"
            ]               
        }, 		
	    {
            version: "v1.6.1",
            changes: [
				"Added functionality to mute individual group chats",
				"Added option to change the background image of the chat (in Preferences)",
				"Attempt to solve the problem of emojis on iPhone (please try it and tell me)",
				"Changed speech bubbles",
				"Updated Chinese, Italian and Netherlands translations",
				"Solved some other minor bugs"
            ]               
        }, 	
	    {
            version: "v1.6.0",
            changes: [
                "Fixed last registration problem (fail-old-version)",
				"Send audio memos using microphone button in chat view",
				"Added support for broadcast messages",
				"Send/forward message to multiple recipients",
				"Export chat messages into html file",
				"See online status in contacts list",
				"Solved some other minor bugs",
            ]               
        }, 
	    {
            version: "v1.5.6",
            changes: [
                "Fixed last registration problem (fail-old-version)"
            ]               
        }, 
                {
            version: "v1.5.5",
            changes: [
                "Fixes the problem getting the sms/voice code."
            ]               
        }, 
	     {
            version: "v1.5.4",
            changes: [
                "Fixed last registration problem (fail-old-version)"
            ]               
        }, 
	    {
            version: "v1.5.3",
            changes: [
                "Fixed media upload problem",
                "Delete gesture in chats list only delete the chat from the list",
                "Added menu item 'delete all messages from chat' in chat view",
                "Added menu item 'delete all messages from all chats' in chats list view"
            ]               
        },  
        {
            version: "v1.5.2",
            changes: [
                "Show all availabe profile pictures in contacts list",
                "Fixed update contacts status"
            ]               
        },  	
        {
            version: "v1.5.1",
            changes: [
                "Fixed fail-old-version problem",
                "New improved chat header",
                "Updated italian and chinese translations",
                "NOTE: due to a Whatsapp change, contacts status are not updated"
            ]               
        },	
        {
            version: "v1.5",
            changes: [
                "New registration method (v2)",
                "Integration with  Sony Ericsson MBW-150",
                "Send message using forward gesture on gesture area",
                "Added remove account option in Account Configuration",
                "Added purchase subscription option in Account Configuration",
                "Added forward to group/forward to contact options in message popup menu",
                "Upon firing a notification sound, wait 2 seconds until playing the next"
            ]               
        },	
        {
            version: "v1.4.3",
            changes: [
                "Fixed plugin error problem when receiving picture notification"
            ]               
        },	
		{
			version: "v1.4.2",
			changes: [
				"New voice method to request the registration code",
				"Fixed picture preview problem",
				"Fixed plugin error when sending a picture",
				"Plugin recompiled in order to support Pixi processor",
				"Fixed the visualization problem of some emojis",
				"Fixed the timestamp problem of offline messages",
				"Enabled log ERROR level in the plugin for debugging puposes",
				"Some other minor changes"
			]				
		},
		{
			version: "v1.4.1",
			changes: [
				"Implemented Whatsapp encrypted protocol 1.2",
				"Therefore the login problem is fixed",
				"Updated Chinese, Italian and Netherlands translations"
			]				
		},
		{
			version: "v1.4.0",
			changes: [
				"Added change status feature",
				"Added complete picture support: profile picture, group pictures and contact pictures",
				"Added new emoji collection",
				"Fixed some minor bugs"
			]				
		},
		{
			version: "v1.3.3",
			changes: [
				"Workaround for last login problem",
			]				
		},
		{
			version: "v1.3.2",
			changes: [
				"Shows notification when a new version is available",
				"German translation corrected",
				"Added Russian translation",
				"Sends unavailable message when app is inactive, in background or minimized",
				"Fixed issue with messages not coming in background mode"
			]				
		},
		{
			version: "v1.3.1",
			changes: [
				"Fixed contact load problem",
				"Updated german and italian translations"
			]				
		},
		{
			version: "v1.3.0",
			changes: [
				"Fixed registration problem (fail-old-version)",
				"Added reduce image before sending in preferences",
				"Added contacts phone types to import in preferences",
				"Fixed Chinese translation",
				"Added French translation",
				"Added change language in preferences"
			]
		},
		{
			version: "v1.2.0",
			changes: [
				"Chinese translation (thanks to tonyw)",
				"Updated scaled Pre3 icons (thanks to virox)",
				"Added contacts status (in contacts list)",
				"Fixed image thumb and image viewer (Pre3)",
				"Other minor bugs fixed"
			]
		},
		{
			version:"v1.1.0",
			changes: [
				"German, italian, netherlands translations (thanks to Sonic-NKT, virox, cainvommars, DMeister, wMark90)",
				"Problem importing all contacts fixed (thanks to capepe)",
				"Pre3 1.5 folder with MojoWhatsup scaled icon (thanks to virox)",
				"Orange/Ctrl + Enter sends message",
				"Removed nojail statement",
				"Fixed problem with screen orientation when returning from image viewer",
				"Message font size included in preferences",
				"Fixed some other minor bugs"
			]
		},
		{
			version:"v1.0.0",
			changes: [
				"Public release!",
				"Max phone length corrected in account configuracion."
			]
		}
	];
}

ChangelogAssistant.prototype.setup = function() {
    if (_chatsAssistant != null && _appAssistant.isTouchPad()) {
        var menuModel = {
            visible : true,
            items : [{
                icon : "back",
                command : "close"
            }]
        };

        this.controller.setupWidget(Mojo.Menu.commandMenu, this.attributes = {
            spacerHeight : 0,
            menuClass : 'no-fade'
        }, menuModel);
    }    
    
	var menuAttr = {omitDefaultItems: true};
  	var menuModel = {
    	visible: true,
    	items: [
      		{label: $L("Back"), command: 'close'}
    	]
  	};
	this.controller.setupWidget(Mojo.Menu.appMenu, menuAttr, menuModel);
	
	this.controller.get("changelogTitle").innerHTML = $L("Change Log");
	var log = "";
	for(var i=0; i<this.changelog.length; i++) {
		log += Mojo.View.render({object:{version:this.changelog[i].version},
				template: "changelog/header"});
		log += "<ul>";
		for(var j=0; j<this.changelog[i].changes.length; j++) {
			log += "<li>" + this.changelog[i].changes[j] + "</li>";
		}
		log += "</ul>";
	}
	log += "<br>";
	this.controller.get("changelog").innerHTML = log;
	this.controller.get("footer").innerHTML = Mojo.Controller.appInfo.copyright;
};

ChangelogAssistant.prototype.activate = function(event) {
	
};

ChangelogAssistant.prototype.handleCommand = function(event) {
	if(event.type == Mojo.Event.command) {
		if(event.command == "close") {
			this.controller.stageController.popScene();
		}
	}
};

ChangelogAssistant.prototype.deactivate = function(event) {

};

ChangelogAssistant.prototype.cleanup = function(event) {
};
