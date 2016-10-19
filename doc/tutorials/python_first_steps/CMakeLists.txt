project(tut_python_first_steps)
cmake_minimum_required(VERSION 3.5)

if (SMTK_ENABLE_TESTING AND SMTK_ENABLE_PYTHON_WRAPPING AND SMTK_DATA_DIR)

  smtk_add_test_python(tut-python_first_steps first_steps.py
      ${SMTK_DATA_DIR}/model/2d/smtk/test2D.json
  )
endif()
