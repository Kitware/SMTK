project(tut_python_first_steps)
cmake_minimum_required(VERSION 3.12)

if (SMTK_ENABLE_TESTING AND SMTK_ENABLE_PYTHON_WRAPPING AND SMTK_DATA_DIR)

  smtk_add_test_python(tut-python_first_steps first_steps.py
      ${SMTK_DATA_DIR}/model/2d/smtk/test2D.json
  )
  set_tests_properties(tut-python_first_steps PROPERTIES SKIP_RETURN_CODE 125)
endif()
