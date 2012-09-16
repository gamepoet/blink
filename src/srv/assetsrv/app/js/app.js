App = Em.Application.create({
  ready: function() {
    // prevent the browser-default drop handler so I can have custom drop zones
    // anywhere in the document and the user can't accidentally navigate away to
    // a local file
//    $(document).bind('drop dragover', function (e) {
//      e.preventDefault();
//    });

    App.pubsub = new Faye.Client('http://localhost:5000/faye');

    App.view = App.MainView.create();
    App.view.appendTo('body');

    $.getJSON('/assets/texture')
    .success(function(data) {
      _.each(data, function(item) {
        App.texturesController.pushObject(App.Texture.create(item));
      });
    });

    App.pubsub.subscribe('/session', function(msg) {
      data = JSON.parse(msg);
//      console.log(data);
      App.onSessionUpdated(data);
    });
    App.pubsub.subscribe('/assets/texture/*', function(msg) {
//      console.log('texture msg');
      data = JSON.parse(msg);
//      console.log(data);
      App.onTextureUpdated(data);
    });

    this._super();
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

  onTextureUpdated: function(data) {
    var tex_id = data._id;
//    console.log('looking for tex: '+tex_id);
    var tex = App.texturesController.find(tex_id);
    if (tex) {
      tex.onSync(data);
    }
    else {
      // not found? must be new
    }
  },

  sendChanges: function(assetType, id, delta) {
//    console.log('sendChanges: ' + JSON.stringify(delta));
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

// utility function to convert a JSON object hierarchy into dot-delimited object
App.flattenJSON = function(obj, prefix, into) {
  prefix = prefix || '';
  into = into || {};

  for (var key in obj) {
    var val = obj[key];
    if (val && typeof val == 'object' && !(val instanceof Date || val instanceof RegExp)) {
      App.flattenJSON(val, prefix + key + '.', into);
    }
    else {
      into[prefix + key] = val;
    }
  }
  return into;
};

// utility function to log a change to a property of an object
App.logChange = function(obj, idKey, path) {
  var id = obj.getPath(idKey);
  var val = obj.getPath(path);
  console.log('[change] ('+obj.__proto__.constructor+' '+id+') '+path+': '+val);
};

App.texturesController = Em.ArrayController.create({
  content: [],

  selection: null,

  find: function(id) {
    return this.get('content').findProperty('_id', id);
  },
});

App.Texture = Em.Object.extend({
  onHeightChanged: function(sender, key) {
    App.logChange(this, '_id', key);
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          height: this.getPath('metadata.target.default.height')
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.height'),

  onSemanticChanged: function(sender, key) {
    App.logChange(this, '_id', key);
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          semantic: this.getPath('metadata.target.default.semantic')
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.semantic'),

  onWidthChanged: function(sender, key) {
    App.logChange(this, '_id', key);
    App.sendChanges('texture', this.get('_id'), {
      target: {
        default: {
          width: this.getPath('metadata.target.default.width')
        }
      }
    })
    .success(_.bind(function(data) {
      this.onSync(data);
    }, this));
  }.observes('metadata.target.default.width'),

  onSync: function(data) {
//    console.log('Texture.onSync');
    var hash = App.flattenJSON(data);
    for (var key in hash) {
//      console.log('setPath: '+key+' = '+hash[key]);
      this.setPath(key, hash[key]);
    }
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
