App = Em.Application.create({
  ready: function() {
    // prevent the browser-default drop handler so I can have custom drop zones
    // anywhere in the document and the user can't accidentally navigate away to
    // a local file
//    $(document).bind('drop dragover', function (e) {
//      e.preventDefault();
//    });

    App.view = App.MainView.create();
    App.view.appendTo('body');

    $.getJSON('/assets/texture')
    .success(function(data) {
      _.each(data, function(item) {
        App.texturesController.pushObject(App.Texture.create(item));
      });
    });

    this.sessionPoll();

    this._super();
  },

  sessionPoll: function() {
    setTimeout(function() {
      $.getJSON('/session')
      .success(function(data) {
        App.onSessionUpdated(data);

        // poll again
        if (!App.stopPolling) {
          App.sessionPoll();
        }
      });
    }, 3000);
  },

  onSessionUpdated: function(data) {
    var selection = data.selection;

    var textures = App.texturesController.get('content');
    var texObj = _.find(textures, function(item) {
      if (selection == item.get('filename')) {
        return true;
      }
      return false;
    });

    App.texturesController.set('selection', texObj);
  },

  sendChanges: function(assetType, id, delta) {
    console.log('sendChanges: ' + JSON.stringify(delta));
    var data = {
      delta: delta
    };

    return $.ajax('/assets/'+assetType+'/'+id, {
      ccontentType: 'application/json',
      type: 'PUT',
      data: JSON.stringify(data),
    });
  },
});

App.texturesController = Em.ArrayController.create({
  content: [],

  selection: null,
});

App.Texture = Em.Object.extend({
  onHeightChanged: function() {
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          height: this.get('metadata').target.default.height
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.height'),

  onSemanticChanged: function() {
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          semantic: this.get('metadata').target.default.semantic
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.semantic'),

  onWidthChanged: function() {
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          width: this.get('metadata').target.default.width
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.width'),

  onSync: function(data) {
    console.log('Texture.onSync');
    this.set('metadata.target.default.height', data.height);
    this.set('metadata.target.default.width', data.width);
    this.set('metadata.target.default.semantic', data.semantic);
  },
});

App.MainView = Em.View.extend({
  templateName: 'main',
});

App.TextureListView = Em.View.extend({
  classNames: ['texture-list'],
  templateName: 'texture-list',

  didInsertElement: function() {
    this.$().fileupload({
      type: 'POST',
      url:  '/assets/texture',
    });
  },
});

App.EditableTextView = Em.View.extend({
  templateName: 'editable-text',

  // the value
  valueBinding: null,

  didInsertElement: function() {
    // Ember won't let me assign multiple actions to the text field, so just do
    // it here with jquery.
    var input = this.$input();
    input.on('focusout', _.bind(function(evt) {
      this.inputFocusOut(evt);
    }, this));
    input.on('keydown', _.bind(function(evt) {
      this.inputKeyDown(evt);
    }, this));

    this.editing = false;
  },

  beginEdit: function() {
    this.editing = true;

    this.$().addClass('editing');
    var input = this.$input();
    input.val(this.filterGet(this.get('value')));
    input.focus();
    input.select();
  },

  cancelEdit: function() {
    if (this.editing) {
      this.$().removeClass('editing');

      this.editing = false;
    }
  },

  endEdit: function() {
    if (this.editing) {
      this.$().removeClass('editing');
      this.set('value', this.filterSet(this.$input().val()));

      this.editing = false;
    }
  },

  // gets the input dom node
  $input: function() {
    return this.$('.edit input');
  },

  // override this to filter the input data on get
  filterGet: function(val) {
    return val;
  },

  // override this to filter the input data on set
  filterSet: function(val) {
    return val;
  },

  //
  // event handlers
  //

  displayClick: function(evt) {
    this.beginEdit();
    evt.stopPropagation();
  },

  inputKeyDown: function(evt) {
    if (evt.keyCode == 13) {
      // enter
      this.endEdit();
    }
    else if (evt.keyCode == 27) {
      // escape
      this.cancelEdit();
    }
  },

  inputFocusOut: function(evt) {
    this.endEdit();
  },
});

App.EditableTextIntView = App.EditableTextView.extend({
  filterGet: function(val) {
    return val.toString();
  },
  filterSet: function(val) {
    return parseInt(val);
  },
});

App.TextureListItemView = Em.View.extend({
  classNames: ['texture-list-item'],
  classNameBindings: ['selected'],

  selected: function() {
    var ctx = this.get('templateContext');
    return ctx == App.texturesController.get('selection');
  }.property('App.texturesController.selection'),

  click: function(evt) {
    var ctx = this.get('templateContext');
    data = {
      delta: {
        selection: ctx.get('filename'),
        selection_id: ctx.get('_id'),
      },
    };

    $.ajax('/session', {
      contentType: 'application/json',
      type: 'PUT',
      data: JSON.stringify(data),
    })
    .success(function(data) {
      console.log('/session: OK');
      App.onSessionUpdated(data);
    });
  },
});
