# Data dictionary - v1.1 / schema 2
14 tables; INTEGER IDs, UTC seconds and integer hundredths of a mark.
SQL CHECK clauses and triggers are authoritative in migrations/*.sql.

## admin_events
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| actor_id | INTEGER | Yes | - | users.id |
| occurred_at | INTEGER | Yes | - | - |
| entity_type | TEXT | Yes | - | - |
| entity_key | TEXT | Yes | - | - |
| action | TEXT | Yes | - | - |
| old_value | TEXT | No | - | - |
| new_value | TEXT | Yes | - | - |
| reason | TEXT | Yes | - | - |

## assessment_events
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| submission_id | INTEGER | Yes | - | submissions.id |
| revision | INTEGER | Yes | - | - |
| actor_id | INTEGER | Yes | - | users.id |
| occurred_at | INTEGER | Yes | - | - |
| action | TEXT | Yes | - | - |
| old_raw_units | INTEGER | No | - | - |
| new_raw_units | INTEGER | Yes | - | - |
| old_final_units | INTEGER | No | - | - |
| new_final_units | INTEGER | Yes | - | - |
| old_feedback | TEXT | No | - | - |
| new_feedback | TEXT | Yes | - | - |
| old_phase | TEXT | Yes | - | - |
| new_phase | TEXT | Yes | - | - |
| reason | TEXT | Yes | - | - |

## assessments
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| submission_id | INTEGER | Yes | - | PK 1; submissions.id |
| version_no | INTEGER | Yes | - | submission_versions.version_no |
| raw_units | INTEGER | Yes | - | - |
| final_units | INTEGER | Yes | - | - |
| late_days | INTEGER | Yes | - | - |
| effective_deadline | INTEGER | Yes | - | - |
| feedback | TEXT | Yes | - | - |
| marker_id | INTEGER | Yes | - | users.id |
| revision | INTEGER | Yes | - | - |
| moderator_id | INTEGER | No | - | users.id |
| moderated_at | INTEGER | No | - | - |
| released_by | INTEGER | No | - | users.id |
| released_at | INTEGER | No | - | - |

## assignments
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| course_id | INTEGER | Yes | - | courses.id |
| creator_id | INTEGER | Yes | - | users.id |
| title | TEXT | Yes | - | - |
| instructions | TEXT | Yes | - | - |
| deadline_utc | INTEGER | Yes | - | - |
| max_mark_units | INTEGER | Yes | - | - |

## class_extensions
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| assignment_id | INTEGER | Yes | - | PK 1; assignments.id |
| deadline_utc | INTEGER | Yes | - | - |
| granted_by | INTEGER | Yes | - | users.id |
| granted_at | INTEGER | Yes | - | - |
| reason | TEXT | Yes | - | - |

## courses
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| code | TEXT | Yes | - | - |
| title | TEXT | Yes | - | - |
| department_id | INTEGER | Yes | - | departments.id |
| active | INTEGER | Yes | 1 | - |

## department_heads
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| department_id | INTEGER | Yes | - | PK 1; departments.id |
| hod_id | INTEGER | Yes | - | users.id |

## departments
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| name | TEXT | Yes | - | - |

## enrolments
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| course_id | INTEGER | Yes | - | PK 1; courses.id |
| student_id | INTEGER | Yes | - | PK 2; users.id |
| active | INTEGER | Yes | 1 | - |

## individual_extensions
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| assignment_id | INTEGER | Yes | - | PK 1; assignments.id |
| student_id | INTEGER | Yes | - | PK 2; users.id |
| deadline_utc | INTEGER | Yes | - | - |
| granted_by | INTEGER | Yes | - | users.id |
| granted_at | INTEGER | Yes | - | - |
| reason | TEXT | Yes | - | - |

## lecturer_courses
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| course_id | INTEGER | Yes | - | PK 1; courses.id |
| lecturer_id | INTEGER | Yes | - | PK 2; users.id |

## submission_versions
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| submission_id | INTEGER | Yes | - | PK 1; submissions.id |
| version_no | INTEGER | Yes | - | PK 2; - |
| received_at | INTEGER | Yes | - | - |
| deadline_at_receipt | INTEGER | Yes | - | - |
| filename | TEXT | Yes | - | - |
| content | BLOB | Yes | - | - |

## submissions
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| assignment_id | INTEGER | Yes | - | assignments.id |
| student_id | INTEGER | Yes | - | users.id |
| phase | TEXT | Yes | 'RECEIVED' | - |
| row_version | INTEGER | Yes | 1 | - |

## users
| Column | SQL type | Required | Default | Key / reference |
|---|---|---|---|---|
| id | INTEGER | Yes | - | PK 1; - |
| username | TEXT | Yes | - | - |
| display_name | TEXT | Yes | - | - |
| password_hash | TEXT | Yes | - | - |
| role | TEXT | Yes | - | - |
| active | INTEGER | Yes | 1 | - |
| must_change_password | INTEGER | Yes | 1 | - |
| failed_attempts | INTEGER | Yes | 0 | - |
| locked_until | INTEGER | No | - | - |
