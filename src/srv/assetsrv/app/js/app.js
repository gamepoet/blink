App = Em.Application.create({
  ready: function() {
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
        App.sessionPoll();
      });
    }, 1000);
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
});

App.texturesController = Em.ArrayController.create({
  content: [],

  selection: null,
});

App.Texture = Em.Object.extend({
});

App.MainView = Em.View.extend({
  templateName: 'main',
});

App.TextureListView = Em.View.extend({
  classNames: ['texture-list'],
  templateName: 'texture-list',
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
