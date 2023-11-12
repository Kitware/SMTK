{
  "conceptual_version": "",
  "id": "e8e813a8-d4f6-493b-bba0-89758476ba3f",
  "links": null,
  "location": "",
  "name": "New Project",
  "operations": {
    "types": []
  },
  "properties": {
  },
  "resources": {
    "resources": [
      {
        "id": "bdf28d45-03fb-4d3a-b87a-52734d7305d7",
        "links": null,
        "location": "resources/attributes-bdf28d45-03fb-4d3a-b87a-52734d7305d7.smtk",
        "name": "elasticity",
        "properties": {
          "unordered_map<uuid, bool>": {
            "smtk.attribute_panel.display_hint": [
              [
                "bdf28d45-03fb-4d3a-b87a-52734d7305d7",
                true
              ]
            ]
          },
          "unordered_map<uuid, double>": {},
          "unordered_map<uuid, int>": {},
          "unordered_map<uuid, long>": {},
          "unordered_map<uuid, map<string, smtk::resource::properties::CoordinateFrame>>": {},
          "unordered_map<uuid, set<int>>": {},
          "unordered_map<uuid, set<smtk::string::Token>>": {},
          "unordered_map<uuid, smtk::resource::properties::CoordinateFrame>": {},
          "unordered_map<uuid, smtk::string::Token>": {},
          "unordered_map<uuid, string>": {
            "project_role": [
              [
                "bdf28d45-03fb-4d3a-b87a-52734d7305d7",
                "attributes"
              ]
            ]
          },
          "unordered_map<uuid, vector<bool>>": {},
          "unordered_map<uuid, vector<double>>": {},
          "unordered_map<uuid, vector<int>>": {},
          "unordered_map<uuid, vector<long>>": {},
          "unordered_map<uuid, vector<string>>": {}
        },
        "type": "smtk::attribute::Resource",
        "version": "3.0"
      }
    ],
    "types": []
  },
  "task_manager": {
    "adaptors": [
      {
        "from": 1,
        "id": 1,
        "to": 3,
        "type": "smtk::task::adaptor::ResourceAndRole"
      }
    ],
    "styles": {
      "operation_view": {
        "operation-panel": {
          "focus-task-operation": true
        }
      },
      "task3_view": {
        "attribute-panel": {
          "attribute-editor": "Elasticity"
        }
      }
    },
    "tasks": [
      {
        "auto-configure": true,
        "id": 1,
        "resources": [
          {
            "max": 1,
            "role": "attributes",
            "type": "smtk::attribute::Resource"
          }
        ],
        "title": "Assign Attribute Resource",
        "type": "smtk::task::GatherResources"
      },
      {
        "id": 2,
        "title": "Plain Task",
        "type": "smtk::task::Task"
      },
      {
        "attribute-sets": [
          {
            "instances": [
              "elasticity"
            ],
            "role": "attributes"
          }
        ],
        "dependencies": [
          2
        ],
        "id": 3,
        "strict-dependencies": true,
        "style": [
          "task3_view"
        ],
        "title": "Edit Attributes",
        "type": "smtk::task::FillOutAttributes"
      }
    ]
  },
  "type": "basic",
  "version": "3.0"
}
